# Holofy

Stylized hologram, transmission, and screen materials for **Unreal Engine 5.8**, plus a runtime component that does the fun stuff: materialize / dematerialize, glitch bursts, signal loss, and smooth morphing between looks.

One shader, ten looks, all blendable.

| Style | Blend | Use it for |
|---|---|---|
| **Classic** | Additive | The blue sci-fi hologram: scanlines, rim glow, rising sweep band |
| **Transmission** | Additive | Incoming holo-call on a shaky signal: static, tearing, RGB split |
| **Tactical** | Additive | Amber war-table projection with a measurement grid |
| **Ghost** | Additive | Faint, breathing, spectral, mostly rim light |
| **Glitch** | Additive | Neon and corrupted, constantly tearing |
| **Hard Light** | Translucent | Near-solid light construct |
| **Holo Screen** | Translucent | Floating sci-fi UI panel showing a texture |
| **CRT Screen** | Masked | Old monitor: curvature, RGB phosphor grid, vignette |
| **Terminal** | Masked | Green phosphor monochrome terminal |
| **Interference** | Translucent | Video feed that's barely holding on |

## Install

1. Clone or copy this repo to `YourProject/Plugins/Holofy`.
2. Regenerate project files and build (it's a C++ plugin).
3. Open the editor. A notification asks to **Generate** the materials. Click it.
   You can re-run this any time from **Tools > Holofy > Generate Hologram Materials**.
4. In the Content Browser, enable **Settings > Show Plugin Content**. The assets are in `Holofy/Materials`.

### Why generate?

Material assets (`.uasset`) are binary, so the plugin builds them from code instead of shipping them. You get:

- `M_Holofy_Additive`, `M_Holofy_Translucent`, `M_Holofy_Masked`: the master materials
- `Presets/MI_Holofy_<Style>`: one material instance per style

Generated assets are normal assets. Commit them to your project's source control as usual.

## Using it

### Fastest: drag a material on

Drag any `MI_Holofy_*` onto a mesh. Done. Tweak the parameters in the instance (they're grouped: Color, Scanlines, Signal, Glitch, Grid, Screen, Content, Materialize, Space).

### Runtime control: Holofy Hologram component

Add **Holofy Hologram** to any actor. At BeginPlay it swaps every mesh on the actor to a live hologram material (use **Target Component Tag** to only affect some meshes).

| Function | What it does |
|---|---|
| `SetStyle(Style, BlendTime)` | Morph to another look over time |
| `SetCustomSettings(Settings, BlendTime)` | Morph to fully custom settings |
| `Materialize(Duration)` / `Dematerialize(Duration)` | Builds in / breaks apart line by line. Fires `OnMaterialized` / `OnDematerialized` |
| `TriggerGlitch(Duration, Intensity)` | One glitch burst. Or tick **Auto Glitch** for random bursts |
| `SetSignalStrength(0..1)` | Lower values add static, dropouts, tearing and a rolling bar |
| `SetContentTexture(Texture)` | What a screen shows |
| `ApplyHologram()` / `RemoveHologram()` | Swap in or restore the original materials |

**Tweaking a preset:** pick a Style, click **Customize Current Style** in the details panel. It copies the preset into Custom Settings and switches to Custom.

Each frame the component only pushes parameters when something changed.

### Screens and displays

Drop a **Holofy Display** actor into the level. It's an upright plane with a Holofy component set to Holo Screen. Set **Hologram > Content Texture** to:

- a regular texture,
- a **Media Texture** for video,
- a **Texture Render Target** fed by a **Scene Capture 2D** for a live camera feed (good for comms),
- a render target with a UMG widget drawn into it.

If the content shows up sideways or mirrored, change **Space > UV Rotation** or **Flip U** in the settings.

### Niagara

The masters have Niagara sprite, ribbon, and mesh usage turned on. Use a `MI_Holofy_*` on a renderer and raise **Vertex Color Amount** so particle color and alpha come through.

### Blueprint helpers (`Holofy` category)

`GetStylePreset`, `LerpStyleSettings`, `ApplySettingsToMaterial`, `CreateHologramMaterial`, `GetBaseMaterial`, `GetPresetMaterial`.

## Tips

- **Coordinate Space**
  - `Local` (the default for 3D holograms): effects run up the object's local Z and are measured in meters.
  - `UV`: for screens.
  - `Screen`: for camera-overlay-style effects.
- **Glitch Vertex Offset** shoves geometry slices sideways. It does nothing useful on a 4-vertex plane, which is why the screen presets set it to 0.
- **Additive** never darkens the scene, which is the classic look. Use **Translucent** if the hologram should occlude a bit. **Masked** is opaque, the cheapest option, and the right choice for in-world monitors.
- **Packaging:** the plugin's `Config/DefaultGame.ini` adds `/Holofy/Materials` to *Directories to Always Cook*. The materials are loaded by path, so they wouldn't get cooked otherwise. If your setup ignores plugin config, add that directory under **Project Settings > Packaging**.
- **Substrate projects:** the masters use the classic Unlit inputs, which Substrate converts automatically. If they look wrong right after generating, restart the editor so the conversion runs on load.

## Extending

All the shading lives in `Shaders/Private/Holofy.ush`, a plain HLSL file. Edits to it are picked up the next time the materials compile (restart the editor or regenerate).

To add a new parameter:

1. Add a field to `FHolofyStyleSettings` in `Source/Holofy/Public/HolofyTypes.h`.
2. Register it in `ForEachFloat` in `HolofyTypes.cpp`.
3. Add a field **with the exact same name** to `FHolofyParams` in `Holofy.ush` and use it.
4. Run **Tools > Holofy > Generate Hologram Materials**.

## Layout

```
Holofy.uplugin
Config/DefaultGame.ini            cook settings
Shaders/Private/Holofy.ush        all the shading
Source/HolofyShaders/             maps /Plugin/Holofy -> Shaders (PostConfigInit)
Source/Holofy/                    runtime: settings, presets, component, display actor
Source/HolofyEditor/              generates the material assets + Tools menu
```
