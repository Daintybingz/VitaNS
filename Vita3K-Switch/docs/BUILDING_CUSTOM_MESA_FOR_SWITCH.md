# Building Custom Mesa/EGL for Nintendo Switch (No Desktop/Nouveau)

This guide explains how to build a custom Mesa/EGL for Nintendo Switch on Ubuntu, ensuring that no desktop Nouveau or GLX support is included. This is necessary to avoid linker errors when building Switch homebrew projects that use GLES2/EGL.

---

## **1. Prepare Your Ubuntu Environment**

- Use a fresh Ubuntu install (bare metal or VM).
- Update your system:

```sh
sudo apt update && sudo apt upgrade -y
```

---

## **2. Install Build Dependencies**

```sh
sudo apt install -y git meson ninja-build python3-mako python3-pip python3-setuptools python3-wheel pkg-config bison flex libexpat1-dev
```

*You may need additional dependencies depending on your configuration. For a minimal build, the above is usually sufficient.*

---

## **3. Clone Mesa Source**

```sh
git clone https://gitlab.freedesktop.org/mesa/mesa.git
cd mesa
```

---

## **4. Configure Mesa for Switch (No Desktop/Nouveau)**

```sh
meson setup builddir \
  -Dplatforms=surfaceless \
  -Dgallium-drivers= \
  -Dvulkan-drivers= \
  -Dglx=disabled \
  -Ddri-drivers= \
  -Degl-native-platform=surfaceless \
  -Dshared-glapi=true \
  -Dgles1=false \
  -Dgles2=true \
  -Dopengl=false \
  -Dgbm=false \
  -Dllvm=false
```

- This disables all desktop drivers and platforms, and only enables surfaceless EGL and GLES2.
- Adjust options as needed for your specific Switch toolchain or backend.

---

## **5. Build Mesa**

```sh
ninja -C builddir
```

- The resulting libraries will be in `builddir/src/egl/` and `builddir/src/gallium/targets/egl/` (and possibly others).

---

## **6. Package the Libraries**

```sh
cd builddir
# Adjust the paths below as needed for your build output
 tar czvf mesa-switch-egl.tar.gz src/egl/libEGL.a src/gallium/targets/egl/libGLESv2.a
```

---

## **7. Use in Your Project or CI**

- Upload `mesa-switch-egl.tar.gz` to a location accessible by your build system (e.g., GitHub release, artifact, or direct download).
- In your build or CI workflow, extract and copy these libraries to your Switch portlibs directory (e.g., `/opt/devkitpro/portlibs/switch/lib/`).

Example (in CI):

```sh
tar xzvf mesa-switch-egl.tar.gz
sudo cp src/egl/libEGL.a /opt/devkitpro/portlibs/switch/lib/
sudo cp src/gallium/targets/egl/libGLESv2.a /opt/devkitpro/portlibs/switch/lib/
```

---

## **8. Build Your Switch Project as Usual**

Your project should now link against your custom Mesa/EGL build, avoiding all desktop/Nouveau linker errors.

---

**If you encounter issues or need to enable additional features, consult the Mesa documentation or adjust the meson options accordingly.** 