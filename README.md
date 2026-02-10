# Atmo Desktop Framework (NSE)

Atmo is a Qt style plugin (`QStyle`) and optional KWin decoration for Plasma 6, using a text-based theme/config system (`NSE.conf`).

This branch is modernized for:
- Qt 6
- KDE Frameworks 6
- Plasma 6 / KDecoration3
- Wayland-first behavior with optional X11 fallback
- HiDPI and fractional scaling correctness

## Components

- `atmo` style plugin: Qt style plugin key `Atmo`
- `atmodecoration` (optional): KWin decoration plugin (KDecoration3)
- `kcm_atmo` (optional): System Settings KCM entry
- `nsesettings`: shared NSE config library
- `atmo_config`: CLI helper for NSE config
- `atmo_manager`: optional GUI manager

## Configuration Model

Atmo keeps the text-based NSE configuration workflow:
- User config: `~/.config/NSE/NSE.conf`
- Installed default template: `/usr/share/atmo/NSE.conf`

Defaults are auto-provisioned from the installed template when the user file is missing.

## Build Dependencies (Ubuntu/Kubuntu 26.04-era)

Required baseline:
- `build-essential`
- `cmake`
- `ninja-build`
- `extra-cmake-modules`
- `qt6-base-dev`
- `libkf6windowsystem-dev`

Recommended for full build:
- `qt6-tools-dev`
- `qt6-tools-dev-tools`
- `libkf6coreaddons-dev`
- `libkf6config-dev`
- `libkf6configwidgets-dev`
- `libkf6kcmutils-dev`
- `libkf6i18n-dev`
- `libkf6guiaddons-dev`

Optional features:
- KWin decoration: `libkdecorations3-dev`
- DBus menu integration: `dbusmenu-qt6` CMake package (if available in your distro; feature auto-disables when missing)
- X11 fallback path: `libxcb1-dev`, `libx11-dev`

Package names can vary slightly across derivatives.

## Build

```bash
cmake -S . -B build -GNinja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DATMO_BUILD_DECORATION=ON \
  -DATMO_BUILD_KCM=ON \
  -DATMO_ENABLE_X11_FALLBACK=ON \
  -DBUILD_MANAGER=ON

cmake --build build -j"$(nproc)"
sudo cmake --install build
```

Useful toggles:
- `-DATMO_BUILD_DECORATION=OFF` to skip KDecoration3 plugin
- `-DATMO_ENABLE_X11_FALLBACK=OFF` for strict Wayland-only runtime paths
- `-DATMO_BUILD_KCM=OFF` to skip System Settings module

## Install Locations

- Style plugin: `${KDE_INSTALL_QTPLUGINDIR}/styles`
- KWin decoration plugin: `${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration3` (if built)
- KCM plugin (System Settings): `${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings` (if built)
- KCM plugin (KDecoration3 hook): `${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration3.kcm` (if built)
- KCM metadata: `${KDE_INSTALL_KSERVICESDIR}`
- Default config template: `${KDE_INSTALL_DATADIR}/atmo/NSE.conf`

## Enabling in Plasma 6

- Application Style: `System Settings -> Colors & Themes -> Application Style -> Atmo`
- Window Decoration (if built): `System Settings -> Colors & Themes -> Window Decorations -> Atmo`
- KCM (if built): appears under System Settings modules as Atmo

## Wayland and HiDPI Notes

- Wayland is the primary runtime path.
- X11-only behavior is runtime-gated and only used when X11 is actually available.
- Core style rendering does not require X11 on Wayland.
- HiDPI-sensitive metrics were moved to DPI-aware helpers (`Ops::dpiScaled`) in key painting/layout paths.
- Pixmap caches for frequently-rendered primitives now include device pixel ratio in cache keys.

## Debug Logging

Set `ATMO_DEBUG=1` to enable runtime debug logs:

```bash
ATMO_DEBUG=1 systemsettings
```

## Smoke Test Checklist

1. Build with Qt6/KF6 and install.
2. In Plasma 6 Wayland session:
   - Select `Atmo` in Application Style.
   - Verify widgets/toolbars/tabs render and react correctly.
3. If decoration built:
   - Select `Atmo` in Window Decorations.
   - Verify button interactions and title rendering.
4. Verify `NSE.conf` compatibility:
   - Existing keys continue to load from `~/.config/NSE/NSE.conf`.
5. Test scaling at 125%, 150%, 175%:
   - Title/buttons/shadows remain crisp.
   - No clipped or tiny controls.
6. Optional X11 fallback check:
   - Start an X11 session and verify fallback-only features still behave.

## Known Limitations

- Some legacy X11-native effects (that depend on X11 window IDs/properties) intentionally degrade on native Wayland instead of forcing XWayland assumptions.
- DBusMenu titlebar integration remains backend/desktop dependent and may be unavailable in some Wayland setups.
