# Atmo Desktop Framework (NSE)

An elegant Qt 5 widget style and KWin decoration designed for SynOS. Atmo brings the New Syndromatic Experience (NSE) to KDE/Qt apps with a unified toolbar/titlebar (UNO), refined shadows and gradients, and a highly configurable, text‑based theme system.

Developed and maintained by Syndromatic Ltd. Derived from “styleproject” (2013, Robert Metsaranta), modernized for Qt 5.

---

## ✨ Highlights

- **Qt 5 style plugin:** Ships as a standard `QStyle` plugin named `Atmo`.
- **KWin decoration:** Optional decoration module (KDecoration2) for a unified look.
- **UNO:** Unified titlebar + toolbar area with optional translucency.
- **Content‑aware toolbars (experimental):** Toolbars can respond to underlying content.
- **Animated interactions:** Hover states, mouseover animations, and polished shadows.
- **Text‑file configuration:** Human‑readable `NSE.conf` with extensive options for buttons, inputs, tabs, sliders, shadows, and more.
- **KF5 integration:** Uses `KF5::WindowSystem`; supports X11/XCB for shadows and effects.

> Note: Wayland support is limited where code depends on X11 (`QX11Info`/XCB). For best results run on Plasma/X11. This will be fixed in the future with SynOS 2 "Juniper"

---

## 📦 Dependencies

Tested on SynOS Canora Beta 2 (Ubuntu/Debian base) with Qt 5.

- Required (Debian/Ubuntu):
  - `build-essential cmake extra-cmake-modules`
  - `qtbase5-dev qttools5-dev qtdeclarative5-dev`
  - `libqt5x11extras5-dev libqt5opengl5-dev libqt5dbus5`
  - `libx11-dev libxcb1-dev libxcb-util-dev`
  - `libkf5windowsystem-dev`
  - `libdbusmenu-qt5-dev` (for mac‑menu integration)
- Optional (for KWin decoration):
  - `libkdecorations2-dev`

---

## 🚀 Build & Install

```
git clone https://github.com/phenom64/atmo-desktop.git
cd atmo-desktop
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j"$(nproc)"
sudo make install
```

What gets installed:
- Qt style plugin `Atmo` into Qt’s `plugins/styles` directory
- Default config at `/usr/share/atmo/NSE.conf`
- `atmo_config` helper CLI into `/usr/bin`
- KWin decoration module (if KDecoration2 is present)

Uninstall:
- From the build directory: `sudo make uninstall`

---

## 🧩 Enabling Atmo in KDE

- Widget Style: open System Settings → “Application Style” → pick `Atmo`.
- Window Decoration: System Settings → “Window Decorations” → pick `Atmo` (requires KDecoration2; only shows if the decoration module was built).

If the style does not appear, see Troubleshooting below.

---

## ⚙️ Configuration (NSE.conf)

Atmo reads settings from `~/.config/NSE/NSE.conf`.

- First run: The project auto‑provisions `~/.config/NSE/NSE.conf` from `/usr/share/atmo/NSE.conf` if the user config is missing.
- Edit directly with your editor, or use the helper:

```
# Write defaults (if you want to reset/seed values)
atmo_config --writedefaults

# Open the active config in your default text editor
atmo_config --edit

# Discover variables and get documentation
atmo_config --listvars
atmo_config --printinfo opacity
```

Notable sections (examples):
- **Window decoration (`deco.*`):** Button style, shadow size/roundness, and icon visibility.
- **Push/Tool buttons (`pushbtn.*`, `toolbtn.*`):** Roundness, gradients, shadows, tints.
- **Inputs (`input.*`):** Roundness, shadow, gradient, tint.
- **Tabs (`tabs.*`):** Safari/Chrome styles, roundness, gradients, close button position.
- **UNO (`uno.*`):** Enable/disable, gradient, tint, noise, blur, overlay, orientation.
- **Sliders/Scrollbars/Progress (`sliders.*`, `scrollers.*`, `progressbars.*`):** Size, gradients, groove styles, dot/metallic effects.
- **Shadows (`shadows.*`):** Opacity and illumination for on‑widget and text shadows.

Each key has an inline description in the source (see `config/settings.cpp`) and via `atmo_config --printinfo`.

---

## 🧱 Project Structure

- `atmolib/`: Core rendering, masks, animations, handlers, widgets (the Atmo style’s internal library)
- `config/`: NSE settings library and `atmo_config` CLI
- `kwin/`: KWin decoration (KDecoration2) and plugin metadata
- `NSE.conf`: Default config template installed to `/usr/share/atmo/NSE.conf`
- `README.md`: This document

---

## 🔧 Development Notes

- Style plugin key: `Atmo` (appears in Qt’s style list)
- Namespace: `NSE` (New Syndromatic Experience)
- Former names: `styleproject`/`DSP` → modernized to Atmo/NSE (Qt 5 only)
- X11/XCB are used for certain effects; Wayland is partially supported

Build tips:
- Ensure `extra-cmake-modules` is installed so CMake can find KF5 and XCB.
- To debug plugin discovery: `export QT_DEBUG_PLUGINS=1` then run a Qt app.

---

## 🧪 Troubleshooting

- **Style not listed under Application Style:**
  - Verify the plugin installed to Qt’s plugin path: `qmake -query QT_INSTALL_PLUGINS`
  - Ensure `libatmo.so` exists under `…/plugins/styles/`
  - Try `QT_DEBUG_PLUGINS=1 systemsettings5` to check plugin load messages

- **Decoration not listed:**
  - Make sure `libkdecorations2-dev` was present at build time (module is optional)
  - Reconfigure and rebuild after installing it

- **No translucency/shadows on Wayland:**
  - Effects using X11/xcb may not apply on Wayland sessions
  - Use an X11 session for complete feature support

---

## 🗺️ Roadmap

- Continue Wayland parity where possible
- Tidy deprecated Qt 5 APIs and smooth migration path to Qt 6
- Expand presets and polish default palette/icon theme handling

---

## 🙏 Credits

- Original base: **styleproject** (c) 2013, Robert Metsaranta — Thank you for the foundation and inspiration.
- Atmo/NSE: (c) 2025 Syndromatic Ltd.

This project is distributed under the **GNU General Public License, version 2 or later (GPL‑2.0‑or‑later)**. See the per‑file headers and top‑level license notices.

---

## 🌐 Links

- Syndromatic: https://syndromatic.com
