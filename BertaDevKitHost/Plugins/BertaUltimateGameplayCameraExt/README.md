# BertaUltimateGameplayCameraExt

Optional UE 5.8 Runtime integration for projects with a licensed local Ultimate Gameplay Camera installation. This plugin is disabled by default and contains no UGC source, content, or assets.

Add `UBertaUGCCameraCycleComponent` to the `PlayerController` whose `PlayerCameraManagerClass` uses `AUGC_PlayerCameraManager` or its Blueprint child. Configure the ordered `CameraPresets` array with distinct UGC camera data assets. Bind the project's own input actions to `SelectNextCamera()` and `SelectPreviousCamera()`, or call `SelectCamera(Index)` directly. Adding the component does not activate a preset automatically.

Each operation reads UGC's current data asset. A known current preset determines the starting index; an unrelated active asset (such as a temporary aim camera) blocks selection without changing the stack or index. With no active asset, Next selects the first preset and Previous the last. Re-selecting the active preset succeeds without Pop/Push. A real change pops the current head, then pushes the requested asset so UGC remains responsible for its transition. Null or duplicate presets make the array invalid.

The component maintains local, non-replicated selection state on the controller. It does not bind input, tick, manage possession, or write camera properties. Install Ultimate Gameplay Camera alongside this plugin and enable both only in the consuming project.
