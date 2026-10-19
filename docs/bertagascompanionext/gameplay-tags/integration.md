# Integration with Blueprint, C++, GAS, and GAS Companion

These examples are **illustrative project integrations**. API signatures and the configuration model were checked against local UE 5.8.2 source; the snippets and Blueprint workflows were not executed in Unreal. The catalog installs no example abilities, effects, actors, or cue assets. Establish [registration](architecture.md) first and perform the [manual checks](troubleshooting.md#manual-acceptance-checklist) in your project.

## Selecting a Blueprint property

1. In a project Blueprint, add a variable with type **Gameplay Tag**. Name it for its purpose, for example `InteractionType`.
2. Compile the Blueprint so its defaults are editable. In the variable's default value picker, search for and select `Berta.Interaction.Type.Inspect`.
3. Hover the selected entry to inspect its description. In Gameplay Tag Manager, inspect its source, `BertaInteractionAndInventory.ini`.
4. Use that variable in project logic that evaluates interaction definitions. The selection itself does not create an inspect interaction.

For multiple labels, use a **Gameplay Tag Container** variable. A rifle item definition might select `Berta.Item.Type.Weapon`, `Berta.Weapon.Type.Rifle`, and `Berta.Item.Rarity.Rare`. Keep the item identifier, stack count, and numeric statistics in their own data. The container is metadata; it does not automatically populate an ASC.

## Blueprint queries

Use native **Has Tag**, **Has Any Tags**, and **Has All Tags** container operations. Choose **Exact Match** when the full identifier must be present; otherwise account for child-to-parent matching. A container containing `Berta.Ability.Combat.Attack.Light` satisfies a non-exact query for `Berta.Ability.Combat.Attack`, but not an exact query for that parent. The reverse direction does not match.

When querying an actor's GAS state, obtain the actor's actual ASC and query its owned tags rather than a similarly named Blueprint variable. A plain container you edited in Class Defaults is a separate source. A target that lacks an expected tag provider is an explicit eligibility failure or unsupported case according to the caller's contract, not proof of an empty authoritative world state.

For richer predicates, configure a native Gameplay Tag Query with Any, All, and No expressions. Define what an empty filter means for your feature. Do not use a tag query to replace authoritative distance, line-of-sight, permissions, or transaction validation.

## C++ requests and container checks

Add `GameplayTags` to the consuming module's dependencies: public only if public headers expose its types, private for implementation-only use. Include the types directly. Call this example after the provider module has registered its sources; avoid static initializers.

```cpp
#include "GameplayTagContainer.h"

bool HasLightAttackLabel(const FGameplayTagContainer& Labels)
{
    const FGameplayTag LightAttack = FGameplayTag::RequestGameplayTag(
        FName(TEXT("Berta.Ability.Combat.Attack.Light")));
    return Labels.HasTagExact(LightAttack);
}

bool HasAnyAttackLabel(const FGameplayTagContainer& Labels)
{
    const FGameplayTag Attack = FGameplayTag::RequestGameplayTag(
        FName(TEXT("Berta.Ability.Combat.Attack")));
    return Labels.HasTag(Attack);
}
```

`RequestGameplayTag` looks up a registered name; it does not register it. Its default `ErrorIfNotFound=true` requests a diagnostic when the contract is broken. For a deliberately optional integration use `RequestGameplayTag(Name, false)`, test `IsValid()`, and return an explicit unsupported/missing-provider result. Do not treat an invalid tag as a successful restriction check. Required code should declare/enable the provider dependency so that an absent catalog is caught during setup.

To inspect runtime ownership on GAS:

```cpp
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

bool IsAirborneByProjectPolicy(const UAbilitySystemComponent& ASC)
{
    const FGameplayTag InAir = FGameplayTag::RequestGameplayTag(
        FName(TEXT("Berta.State.Movement.InAir")));
    return ASC.HasMatchingGameplayTag(InAir);
}
```

This answers what the ASC currently knows. It is accurate only if a project movement integration maintains that tag. It does not query native movement, identify the source of ownership, or guarantee that a client's view is authoritative. If exact ASC ownership is needed, collect `GetOwnedGameplayTags` into a container and use `HasTagExact` with a clearly defined query policy.

## Ability identity and activation restrictions

In a Gameplay Ability Blueprint, use Class Defaults to add `Berta.Ability.Movement.Jump` to **Asset Tags (Default AbilityTags)**. UE 5.8 retains legacy AbilityTags terminology in some UI/API metadata; its native read API is `GetAssetTags()`. Do not add the ability identifier to Activation Owned Tags just to make it appear in diagnostics.

For C++ subclasses, UE 5.8 exposes `SetAssetTags` for setting defaults **in the constructor only**; do not mutate deprecated `AbilityTags` directly. For example, inside your project ability constructor, after establishing that the provider registers before this class's CDO can be constructed:

```cpp
FGameplayTagContainer IdentityTags;
IdentityTags.AddTag(FGameplayTag::RequestGameplayTag(
    FName(TEXT("Berta.Ability.Movement.Jump"))));
SetAssetTags(IdentityTags);
```

The snippet replaces that class's identity container; include any required inherited identities deliberately. It is a constructor fragment, not a free function. Blueprint defaults are usually the simplest option for a Blueprint-authored ability. Calling `SetAssetTags` at runtime is outside its documented contract.

To block an ability while stunned, add `Berta.State.Control.Stunned` to its **Activation Blocked Tags**. This is a native GAS check against the owner's tags. Your condition system must actually grant and remove the stun tag, and your authority/prediction policy must be correct. The catalog alone does neither. Do not use the ability identifier as proof that a stun or a jump is currently active.

Use **Activation Owned Tags** only for a condition whose intended lifetime really is the active ability instance. If `Berta.State.Combat.Attacking` is owned by that instance, configure it there and verify cancellation cleanup. If movement or a weapon controller owns a longer-lived condition, maintain it from that authority instead. Do not give both systems independent authority over the same predicate. Native activation-owned tag replication depends on GAS configuration; inspect the relevant `AbilitySystemGlobals` setting and test the network behavior you need.

## Temporary state through a Gameplay Effect

For an effect-driven stun, create a project Gameplay Effect with a duration and add the component whose native class is `UTargetTagsGameplayEffectComponent`, displayed as **Grant Tags to Target Actor**. Add `Berta.State.Control.Stunned` to that component's **Add Tags** configuration. Apply the effect through your ordinary validated GAS application path. Its active lifetime contributes the granted target tag; expiry/removal ends that effect's contribution.

Do not follow older examples that assign removed/deprecated inherited tag fields directly. The component owns this configuration in UE 5.8. An Instant effect has no lasting active interval for maintaining a temporary state. A tag still does not implement the stun: affected abilities need activation requirements, and movement/input restrictions need their own explicit policy. Stacking, immunity, cleansing, and prediction belong to the effect and application design. Avoid manual tag removal that accidentally contradicts another still-active effect.

## Gameplay Event

After the project confirms a jump occurrence, it can notify an actor's accessible ASC:

```cpp
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"

void NotifyConfirmedJump(AActor* CharacterActor)
{
    if (CharacterActor == nullptr)
    {
        return; // No recipient in this caller's optional notification path.
    }

    const FGameplayTag Jumped = FGameplayTag::RequestGameplayTag(
        FName(TEXT("Berta.Event.Movement.Jumped")));
    FGameplayEventData Payload;
    Payload.EventTag = Jumped;
    Payload.Instigator = CharacterActor;
    Payload.Target = CharacterActor;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        CharacterActor, Jumped, Payload);
}
```

Blueprint exposes **Send Gameplay Event to Actor** with the same concept. Configure a listener or Gameplay Ability event trigger for the selected event. Confirm the actor exposes the intended ASC through its native integration. This helper does not grant that ability or guarantee cross-network delivery. Emit once from the chosen authority/milestone; do not emit independently from input, ability activation, and movement callbacks for the same takeoff. Include useful payload context for other events, such as resolved damage or the affected quest identifier.

## Cooldown

For an optional dash cooldown:

1. Create a duration Gameplay Effect in the game project.
2. Add **Grant Tags to Target Actor** and configure `Berta.Cooldown.Movement.Dash` as a granted target tag.
3. Assign the effect as the dash ability's **Cooldown Gameplay Effect Class** and set the intended duration.
4. Ensure the actual ability path checks and commits cooldowns. Native `UGameplayAbility::GetCooldownTags()` returns its cooldown effect's granted tags, and `CheckCooldown` checks the owner for those restrictions. Custom GSC/project overrides must preserve the intended check.
5. Verify repeat attempts are rejected while the effect is active and succeed after expiry. Verify failed/cancelled-before-commit attempts follow your chosen policy.

Sharing that restriction across several abilities is project configuration. A cooldown tag has no embedded duration and does not automatically block an identically named ability. You may choose no cooldown at all for ordinary jumping. Do not grant cooldown tags as permanent ability metadata or add them to a local container and expect a timer.

## Gameplay Cue

Create a project cue notify/handler associated with `GameplayCue.Berta.Movement.Jump`, ensure its asset location is covered by your GAS cue discovery configuration, then explicitly invoke it from the selected producer or reference it from a Gameplay Effect's cue configuration. A direct ASC execution fragment is:

```cpp
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

void PlayJumpFeedback(UAbilitySystemComponent& ASC, const FVector& Location)
{
    FGameplayCueParameters Parameters;
    Parameters.Location = Location;
    ASC.ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(
        FName(TEXT("GameplayCue.Berta.Movement.Jump"))), Parameters);
}
```

Call it through an intentional authority/prediction path. A one-shot execution differs from adding/removing an active cue; choose and clean up looping cues accordingly. Tag registration does not discover nonexistent assets, play sound, or replicate an authoritative gameplay outcome. The cue can be missing or suppressed while the actual jump remains valid. See the [cue family](catalog/gameplay-cue.md) for that boundary.

## Target filtering

A targeting definition can request `Berta.Targeting.Relation.Enemy` and `Berta.Targeting.Filter.Alive`. The resolver must compute hostility relative to its origin and evaluate the target's life authority. These configuration tags do not mean the target actor literally owns those same filter labels.

If the game exposes its life condition on the target ASC, a small part of its filter might be:

```cpp
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

bool PassesProjectAlivePredicate(const UAbilitySystemComponent* CandidateASC)
{
    if (CandidateASC == nullptr)
    {
        return false; // This filter accepts only targets with the required provider.
    }
    return CandidateASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(
        FName(TEXT("Berta.State.Life.Alive"))));
}
```

This predicate is insufficient for teams, Downed policy, visibility, range, or final application permission. The project explicitly maps requested Filter.Alive semantics to the correct state provider. Native Targeting System and existing GSC TargetTypes can be used, but these labels install no tasks or adapters automatically.

## Inventory and equipment metadata

On a project item definition, store the rifle/type/rarity container described earlier. On a slot definition, use `Berta.Equipment.Slot.MainHand`. On a weapon instance, maintain `Berta.Weapon.State.Reloading` through its weapon controller. Query item metadata for slot compatibility and query the weapon instance for operation state. Do not put every item label on the owning character's ASC: multiple inventory entries require individual identity and scope.

The project supplies actual containers, reflected properties, storage, replication, equip transactions, and validation. Implement `IGameplayTagAssetInterface` when native tag-provider queries benefit your C++ object design; the catalog adds no component or wrapper for it.

## GAS Companion adoption

GSC Gameplay Abilities still inherit native GAS concepts. Select the catalog names in inherited native tag properties and configure GAS Companion's actual Ability Set/Input Action workflow as you already do. Do not replace or edit third-party source. A Berta Input label is not a GSC Input Action assignment, and a Berta Ability identifier does not grant a GSC ability.

Existing extension readiness and loadout diagnostics inspect real grants, effects, input bindings, and owned tags. Use those to inspect the integration after configuring it, without expecting the entire registered dictionary to appear as actor-owned tags. The generic usage finder searches references independently of GAS Companion and can help locate serialized uses before migration.

## Complete manual GA_Jump walkthrough

This walkthrough applies to Nicolas's existing Sandbox setup:

- `BP_ThirdPersonCharacter` derives from `GSCModularCharacter`.
- `GA_Jump` derives from `GSCGameplayAbility`.
- The character already has a working `GSCAbilitySystemComponent`, and Move, Look, and Jump already work.

The Sandbox was **not accessed or modified** for this feature. Apply these steps manually; preserve the already working grant and Enhanced Input configuration.

### Choose the nine roles

| Tag | Role | Adoption decision |
| --- | --- | --- |
| `Berta.Ability.Movement.Jump` | Ability identifier | Recommended on GA_Jump Asset Tags; does not grant or activate it |
| `Berta.State.Movement.Jumping` | Deliberate jump-phase condition | Optional; define the phase and let movement integration own it |
| `Berta.State.Movement.InAir` | Airborne condition | Optional; derive from the native movement authority |
| `Berta.State.Movement.Falling` | Falling movement condition | Optional; document whether it follows native falling mode |
| `Berta.Event.Movement.Jumped` | Accepted takeoff occurrence | Optional; requires an explicit producer/listener |
| `Berta.Event.Movement.Landed` | Landing occurrence | Optional; requires an explicit producer/listener |
| `Berta.Cooldown.Movement.Jump` | Potential restriction | Optional; add only if the game deliberately wants a jump cooldown |
| `GameplayCue.Berta.Movement.Jump` | Takeoff cosmetics | Optional; needs a cue handler/asset and explicit invocation |
| `GameplayCue.Berta.Movement.Land` | Landing cosmetics | Optional; needs a cue handler/asset and explicit invocation |

### Adopt identity first

1. Make the plugin's complete directory available to the Sandbox, enable its prerequisites and the extension, and restart UE 5.8. Verify the three representative tags in Gameplay Tag Manager before editing GA_Jump.
2. Open `GA_Jump` Class Defaults. Add `Berta.Ability.Movement.Jump` to **Asset Tags (Default AbilityTags)**. Compile and save that Blueprint manually. Retain any existing meaningful identity tags unless you deliberately migrate their consumers.
3. Keep its current GSC ability grant and Input Action configuration. Confirm one jump input still reaches the existing ability exactly once. The catalog adds no jump behavior or binding.
4. Inspect the granted spec through your existing diagnostics. The identifier should describe the ability; it need not appear in the ASC's owned-tag list.

### Define one movement source of truth

Use `BP_ThirdPersonCharacter`'s existing native Character Movement information as the movement authority. `UCharacterMovementComponent::IsMovingOnGround()` reports supported ground modes. `IsFalling()` reports the native falling mode and **can be true during upward jump travel**. Flying, Swimming, and custom modes require explicit handling. A simplistic `!IsMovingOnGround()` predicate may be too broad for your intended InAir definition; decide how swimming, flight, custom traversal, and unusual gravity count.

For a minimal normal walking/jumping prototype, derive InAir from native falling mode, and optionally expose Falling with the same native-mode meaning. Define Jumping separately, for example from accepted deliberate takeoff through the end of the ascent phase, using the jump occurrence plus movement information. Do not infer a deliberate jump merely from positive vertical speed or falling mode; a launch pad or walking off a ledge can also create airborne movement. Under custom gravity, use movement-relative rather than assumed world-Z ascent rules.

Put the derived tags behind one project integration owner. In a GAS project that owner can update the character ASC through a deliberately chosen native tag-granting path. If you choose loose tags, inspect and configure their replication semantics explicitly; a plain local add/remove is not a complete multiplayer movement integration. Do not independently set the same movement condition from GA_Jump, Animation Blueprint, and Character Movement callbacks. Animation/UI should consume the derived state or native movement data, not become competing authorities.

### Reconcile transitions, not only the happy path

1. On accepted takeoff, update the project-defined Jumping phase and, if wanted, emit Jumped. Native `ACharacter::OnJumped` is a useful takeoff integration point; an input press or attempted activation is insufficient evidence.
2. Reconcile InAir/Falling from the movement authority on movement-mode changes. Initialize the derived representation when the pawn/ASC becomes available, including possession or respawn, so a missed earlier callback does not leave stale state.
3. End Jumping at the chosen phase boundary. A project may use apex/descent or the whole deliberate airborne interval, but must document one definition. Neither choice should be dictated solely by when GA_Jump ends.
4. On native landing notification, optionally emit Landed and invoke the landing cue. **The movement mode is still Falling during `ACharacter::Landed`/its landed notification** in the inspected UE 5.8 source. Reconcile final supported-state tags after the movement-mode change; immediately re-reading IsFalling inside the landing callback is not proof that landing failed.
5. Handle forced mode changes, teleportation, death, cancellation, swimming entry, and pawn/ASC replacement. Clean up derived conditions through the same owner. Decide whether a cancelled jump ability should stop jump hold behavior while the character remains airborne.

A typical GA_Jump can request native jumping and end before the character lands. InAir and Falling may remain true after the ability's EndAbility. Conversely a long-lived ability may wait through landing. These are valid different lifetimes; do not put airborne state in Activation Owned Tags unless your actual ability lifetime and movement contract intentionally coincide.

### Add optional restrictions and feedback independently

Start without a cooldown if ordinary platform jumping needs none. If desired, configure a duration cooldown effect granting `Berta.Cooldown.Movement.Jump`, assign it to GA_Jump, and verify the actual ability's commit/check path. It does not replace native CanJump rules and must not prevent required cleanup after an airborne cancellation.

Add Jumped/Landed event producers only when a consumer needs them. Choose one producer per occurrence and test duplicate suppression under prediction/correction. Add the two cue implementations only when presentation needs them. Never apply movement or damage from receiving a cue. None of these optional steps is necessary just to classify GA_Jump with its identifier.

### Manually verify the Sandbox result

Check normal jump, held/released input, walking off a ledge, interrupted ability, landing, forced movement-mode change, and respawn. Observe native movement and ASC state together. Confirm the ability can end while airborne state remains valid; Jumped is absent for a mere walk-off; Landed occurs once per chosen landing policy; state cleans up after the actual mode transition. If networking is part of the project, repeat with server/owning client and inspect correction, ownership, and cue duplication. These are pending manual checks, not results established by the static catalog validation.
