# BertaDevKit — Unreal Engine 5.6 C++ Plugin

A personal C++ utility plugin for Unreal Engine 5.6, built to consolidate reusable development tools across projects. The plugin is structured as two modules — a Runtime module with gameplay utilities and debug systems, and an Editor module with asset management and world validation tools — all configurable from Project Settings without touching code.

---

## Tech Stack

|  |  |
| --- | --- |
| **Engine** | Unreal Engine 5.6 |
| **Language** | C++ |
| **IDE** | JetBrains Rider |
| **Coding Standard** | Epic Games C++ Coding Standard |
| **Runtime Dependencies** | Core, CoreUObject, Engine, DeveloperSettings |
| **Editor Dependencies** | UnrealEd, AssetTools, Blutility, EditorScriptingUtilities, Niagara, UMG, AIModule, Foliage, Landscape, PhysicsCore, Slate, SlateCore, ToolMenus, StructUtils, EnhancedInput |

---

## Plugin Structure

```
BertaDevKit/
├── BertaDevKit.uplugin
├── Content/                          ← BP asset derived from BertaAssetNamingActions
└── Source/
    ├── BertaDevKit/                  ← Runtime module
    │   ├── Public/
    │   │   ├── Settings/
    │   │   │   └── BertaDevKitSettings      # UDeveloperSettings — Project Settings config
    │   │   ├── Debug/
    │   │   │   ├── BertaDebugUtils          # Unified debug logging
    │   │   │   ├── BertaDebugDraw           # In-world debug shape drawing
    │   │   │   └── BertaScreenStats         # Persistent on-screen stat panel
    │   │   ├── Math/
    │   │   │   └── BertaMathUtils           # Remap, easing, angular, snap, prediction
    │   │   └── World/
    │   │       └── BertaWorldUtils          # Actor queries, traces, player access, timers
    └── BertaDevKitEditor/            ← Editor module
        └── Public/
            ├── AssetActions/
            │   ├── BertaAssetNamingActions  # Content Browser context menu entry point
            │   ├── BertaAssetNamingUtils    # Prefix map + rename logic
            │   └── BertaAssetAuditor        # Naming convention audit + auto-fix
            ├── Toolbar/
            │   └── BertaEditorToolbar       # Tools menu entries
            └── WorldValidation/
                └── BertaWorldValidation     # Actor validation checks
```

---

## Runtime Systems

### Settings (`UBertaDevKitSettings`)

Extends `UDeveloperSettings` to expose plugin configuration under **Project Settings → Plugins → BertaDevKit**, persisted in `Config/DefaultBertaDevKit.ini`. All runtime systems check their master switch here before executing — disabling a system costs a single `GetDefault<>` call with no downstream overhead.

**Debug toggles:** `bDebugLogEnabled`, `bDebugDrawEnabled`, `bScreenStatsEnabled`.

**World Validation toggles:** `bWorldValidationEnabled`, `bValidateStaticMeshComponents`, `bValidateWorldBounds` (with configurable `WorldBoundsThreshold` in cm), `bValidateLightMobility` (with configurable `ExpectedLightMobility`), `bValidateActorScale`.

Access anywhere via `UBertaDevKitSettings::Get()`, which wraps `GetDefault<>` and returns the CDO — always valid, never null.

---

### Debug Logging (`UBertaDebugUtils`)

`UBlueprintFunctionLibrary` with four `DevelopmentOnly` public functions. All functions accept a `bEnabled` parameter for per-call gating, and respect the `bDebugLogEnabled` master switch.

**Functions:**
- `PrintLog` — prints to screen, Output Log, or both. Accepts a stable `Key` for overwrite behavior.
- `PrintLogWithContext` — prepends `[ObjectName]` from a `DefaultToSelf` `WorldContext`.
- `PrintLogToNamedCategory` — routes to an arbitrary named log category via `FMsg::Logf`.
- `PrintLogToNamedCategoryWithContext` — combines the above two.

Output routing is controlled by `EBertaLogOutput` (`PrintAndLog`, `PrintOnly`, `LogOnly`). Color and verbosity are controlled by `EBertaLogVerbosity` (`Log` → Cyan, `Warning` → Yellow, `Error` → Red). A `default:` with `ensureMsgf` catches unhandled enum values at development time. `Duration` and `Key` parameters are exposed under `AdvancedDisplay` in Blueprint nodes to keep the default pin surface minimal.

---

### Debug Drawing (`UBertaDebugDraw`)

`UBlueprintFunctionLibrary` with twelve `DevelopmentOnly` functions. All respect the `bDebugDrawEnabled` master switch and accept a `Duration` parameter where `<= 0.0f` maps to persistent (`-1.0f` sentinel for `DrawDebug*` functions), handled by a private `ResolveLifeTime` helper.

Three private helpers eliminate per-function boilerplate: `ResolveWorld` (validates context and resolves `UWorld*`), `ResolveLifeTime` (maps duration to `DrawDebug*` convention), `ToFColor` (converts `FLinearColor` to `FColor` for the `DrawDebug*` API).

**Shape functions:** `DrawSphere`, `DrawSphereFromComponent`, `DrawLine`, `DrawBox` (AABB), `DrawBoxFromComponent` (oriented), `DrawCapsule`, `DrawCapsuleFromComponent`, `DrawPoint`, `DrawArrow`, `DrawString`, `DrawCoordinateSystem`, `FlushPersistentShapes`.

`FromComponent` variants extract location, extent, and orientation directly from the component, eliminating boilerplate at the call site. `DrawCoordinateSystem` passes `bPersistentLines = false` as a workaround for a known engine bug where `true` ignores `LifeTime` and prevents flushing.

---

### On-Screen Stats Panel (`UBertaScreenStats`)

`UBlueprintFunctionLibrary` implementing a persistent named stats panel rendered via `GEngine->AddOnScreenDebugMessage` with stable keys derived from `GetTypeHash(FName)`. Each named entry occupies exactly one screen line — calling `Set*` with the same name updates the entry rather than appending a duplicate.

Registration is **lazy**: the `RenderStats` callback binds to `FCoreDelegates::OnBeginFrame` only on the first `Set*` call. A static local flag in `EnsureRegistered()` prevents double-registration. Internal entry storage uses a static local `TMap<FName, FBertaStatEntry>` to avoid the static initialization order fiasco.

`DisplayName` is converted from `FName` to `FString` once on `FindOrAdd`, never per frame. `SetFloat` and `SetVector` use `FCString::Snprintf` with `%.*f` for runtime-dynamic decimal precision, bypassing `TCheckedFormatString` limitations introduced in UE 5.4+.

**Functions:** `SetFloat`, `SetInt`, `SetBool` (auto-colors green/red), `SetString`, `SetVector`, `Remove`, `Clear`.

---

### Math Utilities (`UBertaMathUtils`)

`UBlueprintFunctionLibrary` with fifteen stateless, side-effect-free functions, all `BlueprintThreadSafe`. Safe to call from Animation Blueprints and multithreaded contexts.

**Remap:** `NormalizeToRange`, `RemapClamped`, `RemapUnclamped` — clamping in the former is applied by `NormalizeToRange` constraining the alpha to `[0, 1]` before `FMath::Lerp`, not by an explicit clamp on the output.

**Easing:** `EaseInQuad`, `EaseOutQuad`, `EaseInOutQuad`, `EaseInCubic`, `EaseOutCubic`, `EaseInOutCubic`, `SmoothStep` — cubic Hermite with `f'(0) = f'(1) = 0`, equivalent to GLSL/HLSL `smoothstep`.

**Angular:** `ShortestAngleDelta` (wraps to `[-180, 180]` via `FMath::UnwindDegrees`), `NormalizeAngle180`, `IsAngleInArc` (handles wrap-around via shortest-delta comparison).

**Snap:** `SnapToMultiple`, `RoundToMultiple` (alias for naming clarity), `DistributePointsOnLine` (single point placed at midpoint when `Count == 1`; first and last land exactly on `Start`/`End` for `Count >= 2`).

**Prediction:** `PredictFuturePosition` (constant-velocity kinematic), `ProjectileImpactPoint` (quadratic solver for intersection with a horizontal plane, handles zero-gravity linear case, returns false for no-real-solution and past-only-solution cases).

---

### World Utilities (`UBertaWorldUtils`)

`UBlueprintFunctionLibrary` with ten Blueprint-callable functions covering actor queries, traces, player access, and timers. A private `GetWorldChecked` helper centralizes `WorldContextObject` validation and logs errors with object context on failure. A private `PassesTagFilter` helper uses `NAME_None` as a sentinel for "no filter required".

**Actor Queries:** `GetActorsInRadius`, `GetClosestActorInRadius`, `GetAllActorsOfClass`, `GetFirstActorOfClass` — all iterate via `TActorIterator`, compute distance in squared space to avoid `sqrt`, and support optional tag filtering.

**Traces:** `LineTrace`, `SphereTrace` — single-hit, `bTraceComplex = false`, with `AddIgnoredActors` support.

**Player Access:** `GetPlayerPawn`, `GetPlayerController`, `GetPlayerCameraLocation`, `GetPlayerCameraDirection` — delegate to `UGameplayStatics`; camera functions guard on `PlayerCameraManager` validity and return safe fallback values (`ZeroVector`, `ForwardVector`).

**Timers:** `SetDelayedAction` (one-shot, validates `Delay > 0` and `Callback.IsBound()` before setting), `CancelDelayedAction` (safe to call on invalid or expired handles).

---

## Editor Systems

### Asset Naming Actions (`UBertaAssetNamingActions`)

`UAssetActionUtility` marked `Abstract`, exposing two functions to the Content Browser right-click context menu under **Scripted Asset Actions** via a derived BP asset in `BertaDevKit/Content/`. All logic is delegated to `UBertaAssetAuditor`.

- **Audit Asset Naming** — scans assets in scope and reports naming violations to the Output Log without modifying anything.
- **Fix Asset Naming** — scans assets in scope and renames violators to match naming conventions.

---

### Asset Naming Utils (`UBertaAssetNamingUtils`)

Sole owner of all prefix resolution logic, consumed by both `UBertaAssetNamingActions` and `UBertaAssetAuditor`. Exposes five static methods:

- `GetPrefixMap()` — static local `TMap<UClass*, FString>`, built once on first call.
- `GetOptionalPluginPrefixes()` — static local `TMap<FName, FString>` keyed by class name string, covering classes from optional plugins (GAS) without hard module dependencies.
- `FindPrefixForClass(UClass*, UObject*)` — for loaded Blueprint assets, walks `UBlueprint::ParentClass` to resolve framework and GAS subclasses correctly. For non-Blueprint assets, walks the asset's own class hierarchy.
- `ResolveBlueprintPrefixFromTag(FAssetData)` — resolves prefix for unloaded Blueprint assets by parsing the `ParentClass` Asset Registry tag, avoiding loading assets into memory during audit passes.
- `RenameAssetWithPrefix(UObject*)` — applies the resolved prefix, stripping engine-generated suffixes for `UMaterialInstanceConstant` (`M_`, `_Inst`) and `UAnimMontage` (`_Montage`).

Classes from optional plugins (e.g. `GameplayAbilities`) are resolved via `GetOptionalPluginPrefixes()` by class name string comparison, avoiding hard module dependencies that would prevent the editor from launching on projects where those plugins are disabled.

**Supported prefixes:**

| Class | Prefix |
| --- | --- |
| `UBlueprint` | `BP_` |
| `UAnimBlueprint` | `ABP_` |
| `UUserWidget` | `WBP_` |
| `UStaticMesh` | `SM_` |
| `USkeletalMesh` | `SKM_` |
| `UMaterial` | `M_` |
| `UMaterialInstanceConstant` | `MI_` |
| `UMaterialParameterCollection` | `MPC_` |
| `UTexture2D`, `UTextureCube` | `T_` |
| `UTextureRenderTarget2D` | `RT_` |
| `UAnimSequence` | `AS_` |
| `UAnimMontage` | `AM_` |
| `UBlendSpace`, `UBlendSpace1D` | `BS_` |
| `UAimOffsetBlendSpace`, `UAimOffsetBlendSpace1D` | `AO_` |
| `UParticleSystem` | `PS_` |
| `UNiagaraSystem` | `NS_` |
| `UNiagaraEmitter` | `NE_` |
| `USoundWave` | `SW_` |
| `USoundCue` | `SC_` |
| `UDataTable` | `DT_` |
| `UDataAsset` | `DA_` |
| `UPhysicsAsset` | `PA_` |
| `UUserDefinedEnum` | `E_` |
| `UUserDefinedStruct` | `F_` |
| `AGameModeBase`, `AGameMode` | `GM_` |
| `APlayerController` | `PC_` |
| `ACharacter` | `CH_` |
| `APawn` | `P_` |
| `UBlackboardData` | `BB_` |
| `AAIController` | `AIC_` |
| `UBTDecorator` | `BTD_` |
| `UBTService` | `BTS_` |
| `UBTTaskNode` | `BTT_` |
| `UEnvQuery` | `EQS_` |
| `UEnvQueryContext` | `EQSC_` |
| `UInputAction` | `IA_` |
| `UInputMappingContext` | `IMC_` |
| `UGameplayAbility` *(optional)* | `GA_` |
| `UGameplayEffect` *(optional)* | `GE_` |
| `UGameplayCueNotify_Static` *(optional)* | `GC_` |
| `UGameplayCueNotify_Actor` *(optional)* | `GC_` |

---

### Asset Auditor (`UBertaAssetAuditor`)

Static-interface `UObject` with two entry points: `AuditAssetNaming` (reports violations without modifying assets) and `FixAssetNaming` (renames violators). Both delegate scope resolution to `ResolveAssetScope`, which uses the following priority:

1. **Selected assets** — if any assets are individually selected in the Content Browser.
2. **Active folder** — the folder currently navigated to in the Content Browser directory tree, scanned recursively via the Asset Registry.
3. **Full `/Game/` scan** — fallback when neither of the above yields a scope.

`AuditAssetNaming` resolves prefixes via `ResolveBlueprintPrefixFromTag` for Blueprint assets (no memory load) and `FindPrefixForClass` for non-Blueprint assets. `FixAssetNaming` loads each violating asset and uses `FindPrefixForClass` with the loaded `UBlueprint` object for accurate `ParentClass` walking, falling back to `ResolveBlueprintPrefixFromTag` when the walk yields no match. Results are reported to `LogBertaDevKitEditor` and summarized via `FNotificationInfo` with `CS_Success` / `CS_Fail` completion state.

---

### Editor Toolbar (`FBertaEditorToolbar`)

Non-UObject `F` class owned by `FBertaDevKitEditorModule` via `TUniquePtr`. Registers three entries under `LevelEditor.MainMenu.Tools` in a **BertaDevKit** section:

- **Run Asset Audit** → `UBertaAssetAuditor::AuditAssetNaming()`
- **Fix Asset Naming** → `UBertaAssetAuditor::FixAssetNaming()`
- **Run World Validation** → `UBertaWorldValidation::RunValidation()`

Registration is deferred to `FCoreDelegates::OnPostEngineInit` via a named member callback because `UToolMenus` is not guaranteed to exist at `StartupModule` time. Each entry's `Owner` field is set to `FToolMenuOwner("BertaDevKit")` so that `UnregisterOwner` in `ShutdownModule` correctly cleans up all entries on hot-reload and plugin unload.

---

### World Validation (`UBertaWorldValidation`)

Static-interface `UObject` with a single entry point: `RunValidation`. Iterates all actors in the currently open level via `TActorIterator<AActor>`. Settings are read once at the start to avoid redundant `GetDefault<>` calls per actor. Reports violations to `LogBertaDevKitEditor` and displays a `FNotificationInfo` summary with `CS_Success` / `CS_Fail` state.

**Active checks:**
- **Static Mesh Components** — flags `UStaticMeshComponent` with no mesh asset assigned.
- **World Bounds** — flags actors beyond `WorldBoundsThreshold` on any axis.
- **Light Mobility** — flags light actors whose mobility does not match `ExpectedLightMobility`.
- **Actor Scale** — flags actors with zero or negative scale on any axis.

---

## Getting Started

### Requirements

- Unreal Engine 5.6
- Visual Studio 2022 or JetBrains Rider
- Windows 10/11 (64-bit)

### Setup

1. Clone the repository into your project's `Plugins/` folder:
   ```
   git clone https://github.com/nbertoa/ue5-bertadevkit Plugins/BertaDevKit
   ```
2. Right-click the `.uproject` file and select **Generate Visual Studio project files**.
3. Open the solution and build.
4. Enable the plugin in **Edit → Plugins → BertaDevKit**.
5. Configure systems under **Project Settings → Plugins → BertaDevKit**.

> **Note:** If your project uses the Editor module and has GAS assets, the `GameplayAbilities` plugin must be enabled in your `.uproject`. The plugin itself does not require `GameplayAbilities` — GAS class prefixes are resolved via class name string comparison to avoid hard module dependencies.

---

## Log Categories

| Category | Scope |
| --- | --- |
| `LogBertaDevKit` | Runtime module — world utils, settings, screen stats |
| `LogBertaDebug` | Debug utils and debug draw |
| `LogBertaDevKitEditor` | Editor module — asset auditor, toolbar, world validation |

---

## License

This project is released under the [MIT License](LICENSE).

---

## About

**Nicolás Bertoa** — Unreal Engine developer with 14+ years of professional experience, focused on C++ and gameplay systems.

🌐 [Portfolio](https://nbertoa.wordpress.com) | 🎬 [Demo Reels](https://nbertoa.wordpress.com/demo-reels/)
