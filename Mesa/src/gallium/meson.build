# Copyright © 2017 Dylan Baker
# Copyright © 2017-2018 Intel Corporation

# Minimal Gallium for Nintendo Switch - Software rendering only

# Define missing variables for Switch build
# Gallium drivers
with_gallium_r300 = false
with_gallium_r600 = false
with_gallium_radeonsi = false
with_gallium_nouveau = false
with_gallium_freedreno = false
with_gallium_asahi = false
with_gallium_softpipe = true
with_gallium_drisw_kms = false
with_gallium_kmsro = false
with_gallium_vc4 = false
with_gallium_v3d = false
with_gallium_panfrost = false
with_gallium_etnaviv = false
with_gallium_tegra = false
with_gallium_crocus = false
with_gallium_iris = false
with_gallium_i915 = false
with_gallium_svga = false
with_gallium_virgl = false
with_gallium_lima = false
with_gallium_zink = false
with_gallium_d3d12 = false
with_gallium_opencl = false
with_gallium_rusticl = false
with_gallium_vdpau = false
with_gallium_va = false
with_gallium_xa = false
with_gallium_st_nine = false
with_gallium_st_d3d10umd = false

# Platform support
with_platform_windows = false
with_platform_haiku = false
with_platform_x11 = false
with_platform_wayland = false
with_platform_android = false

# Other features
with_dri = false
with_llvm = false
draw_with_llvm = false
with_tests = false
with_tools = []
with_vulkan_beta = false
with_vulkan_icd_dir = ''

# Intel variables
with_intel_vk = false
with_intel_hasvk = false
with_intel_clc = false
with_intel_vk_rt = false
with_intel_tools = false

# AMD variables
with_amd_vk = false

# Broadcom variables
with_broadcom_vk = false

# Panfrost variables
with_panfrost_vk = false

# GLX/EGL
with_glx = 'disabled'
with_egl = true

# Additional commonly used variables
with_any_opengl = false
with_shared_glapi = false
with_glvnd = false
with_symbols_check = false
with_asm_arch = 'none'
with_sse41 = false
with_platform_apple = false
with_dri_platform = 'none'
system_has_kms_drm = false
freedreno_kmds = []
with_osmesa = false
with_opengl = false
with_swrast_vk = false
with_teflon = false

# Common dependencies (null for Switch build)
null_dep = dependency('', required : false)
dep_libdrm = null_dep
dep_libxml2 = null_dep
dep_clock = null_dep
dep_thread = null_dep
dep_atomic = null_dep
dep_m = null_dep
dep_dl = null_dep
dep_unwind = null_dep
dep_futex = null_dep
dep_network = null_dep
dep_perfetto = null_dep
dep_expat = null_dep
dep_regex = null_dep
dep_android = [null_dep, null_dep, null_dep]
dep_android_mapper4 = null_dep
dep_glvnd = null_dep
dep_omx = null_dep
dep_omx_other = []
dep_va = null_dep
dep_vdpau = null_dep
dep_xv = null_dep
dep_x11_xcb = null_dep
dep_xcb_dri2 = null_dep
dep_xcb_xrandr = null_dep
dep_xcb_xfixes = null_dep
dep_wayland_client = null_dep
dep_wayland_server = null_dep
dep_wayland_egl_headers = null_dep
dep_wayland_drm = null_dep
dep_wayland_protocols = null_dep
dep_lmsensors = null_dep
dep_ws2_32 = null_dep
dep_dxheaders = null_dep
dep_clc = null_dep
dep_valgrind = null_dep
dep_zlib = null_dep
dep_zstd = null_dep
c_msvc_compat_args = []
cpp_msvc_compat_args = []

# Common include directories
inc_util = include_directories('../util')
inc_loader = include_directories('../loader')
inc_gallium = include_directories('.', 'include')
inc_include = include_directories('../../include')
inc_src = include_directories('..')
inc_gallium_aux = include_directories('auxiliary')
inc_gallium_winsys = include_directories('winsys')
inc_gallium_drivers = include_directories('drivers')
inc_gallium_winsys_sw = include_directories('winsys/sw')
inc_mapi = include_directories('../mapi')
inc_gbm = include_directories('../gbm')
inc_compiler = include_directories('../compiler')
inc_nir = include_directories('../compiler/nir')

# Common ideps
idep_xmlconfig = declare_dependency()
idep_mesautil = declare_dependency()
# NIR dependencies are now properly declared in compiler/nir/meson.build
# and will be available here
idep_gtest = declare_dependency()
# Compiler dependency - will be properly defined after subdir('compiler')
libcompiler_dep = declare_dependency()

# Common library variables (empty for Switch build)
libloader = []
libswdri = []
libswkmsdri = []
libwsgdi = null_dep
driver_swrast = declare_dependency()
driver_asahi = declare_dependency()
driver_r300 = declare_dependency()
driver_r600 = declare_dependency()
driver_radeonsi = declare_dependency()
driver_nouveau = declare_dependency()
driver_freedreno = declare_dependency()
driver_vc4 = declare_dependency()
driver_panfrost = declare_dependency()
driver_etnaviv = declare_dependency()
driver_tegra = declare_dependency()
driver_crocus = declare_dependency()
driver_iris = declare_dependency()
driver_i915 = declare_dependency()
driver_svga = declare_dependency()
driver_virgl = declare_dependency()
driver_lima = declare_dependency()
driver_zink = declare_dependency()
driver_d3d12 = declare_dependency()
libgbm = []
libwayland_drm = []
libloader_wayland_helper = []
libmesa = []
libgallium = []
libswhgl = []
libsthgl = []
libgallium_wgl = []
winsys_d3d12_wgl = declare_dependency()
libgalliumvl = []
libgalliumvl_stub = []
libgalliumvlwinsys = []
vlwinsys_deps = []

# Common file variables
files_libgalliumvl = []
files_libgalliumvlwinsys = []

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

sym_config = configuration_data()

foreach d : [[with_gallium_r300 or with_gallium_radeonsi or with_gallium_r600, 'radeon_drm_winsys_create'],
             [with_gallium_radeonsi, 'amdgpu_winsys_create'],
             [with_gallium_nouveau, 'nouveau_drm_screen_create'],
             [with_gallium_freedreno, 'fd_drm_screen_create_renderonly'],
             [with_llvm and with_gallium_radeonsi, 'ac_init_shared_llvm_once']]
  if d[0]
    sym_config.set(d[1], d[1] + ';')
  else
    sym_config.set(d[1], '')
  endif
endforeach

inc_gallium_drivers = include_directories('drivers')
inc_gallium_winsys = include_directories('winsys')
inc_gallium_winsys_sw = include_directories('winsys/sw')

subdir('auxiliary')
subdir('auxiliary/pipe-loader')
subdir('winsys/sw/null')
if with_dri
  subdir('winsys/sw/dri')
else
  libswdri = []
endif
if with_gallium_drisw_kms
  subdir('winsys/sw/kms-dri')
else
  libswkmsdri = []
endif
if with_platform_windows
  subdir('winsys/sw/gdi')
else
  libwsgdi = null_dep
endif
subdir('winsys/sw/wrapper')
if with_platform_haiku
  subdir('winsys/sw/hgl')
endif
if with_gallium_softpipe
  subdir('drivers/softpipe')
  if draw_with_llvm
    subdir('drivers/llvmpipe')
  endif
else
  driver_swrast = declare_dependency()
endif
if with_gallium_asahi
  subdir('winsys/asahi/drm')
  subdir('drivers/asahi')
else
  driver_asahi = declare_dependency()
endif
if with_gallium_r300 or with_gallium_radeonsi or with_gallium_r600
  subdir('winsys/radeon/drm')
endif
if with_gallium_r300
  subdir('drivers/r300')
else
  driver_r300 = declare_dependency()
endif
if with_gallium_r600
  subdir('drivers/r600')
else
  driver_r600 = declare_dependency()
endif
if with_gallium_radeonsi
  subdir('winsys/amdgpu/drm')
  subdir('drivers/radeonsi')
else
  driver_radeonsi = declare_dependency()
endif
if with_gallium_nouveau
  subdir('winsys/nouveau/drm')
  subdir('drivers/nouveau')
else
  driver_nouveau = declare_dependency()
endif
if with_gallium_freedreno
  subdir('winsys/freedreno/drm')
  subdir('drivers/freedreno')
else
  driver_freedreno = declare_dependency()
endif
if with_gallium_vc4
  subdir('winsys/vc4/drm')
  subdir('drivers/vc4')
else
  driver_vc4 = declare_dependency()
endif
if with_gallium_panfrost
  subdir('winsys/panfrost/drm')
  subdir('drivers/panfrost')
else
  driver_panfrost = declare_dependency()
endif
if with_gallium_etnaviv
  subdir('winsys/etnaviv/drm')
  subdir('drivers/etnaviv')
else
  driver_etnaviv = declare_dependency()
endif
if with_gallium_kmsro
  subdir('winsys/kmsro/drm')
else
  driver_kmsro = declare_dependency()
endif
if with_gallium_v3d
  subdir('winsys/v3d/drm')
  subdir('drivers/v3d')
else
  driver_v3d = declare_dependency()
endif
if with_gallium_tegra
  subdir('winsys/tegra/drm')
  subdir('drivers/tegra')
else
  driver_tegra = declare_dependency()
endif
if with_gallium_crocus
  subdir('winsys/crocus/drm')
  subdir('drivers/crocus')
else
  driver_crocus = declare_dependency()
endif
if with_gallium_iris
  subdir('winsys/iris/drm')
  subdir('drivers/iris')
else
  driver_iris = declare_dependency()
endif
if with_gallium_i915
  subdir('winsys/i915/drm')
  subdir('drivers/i915')
else
  driver_i915 = declare_dependency()
endif
if with_gallium_svga
  if not with_platform_windows
     subdir('winsys/svga/drm')
  endif
  subdir('drivers/svga')
else
  driver_svga = declare_dependency()
endif
if with_gallium_virgl
  subdir('winsys/virgl/common')
  if not with_platform_windows
    subdir('winsys/virgl/drm')
    subdir('winsys/virgl/vtest')
  endif
  subdir('drivers/virgl')
else
  driver_virgl = declare_dependency()
endif
if with_gallium_lima
  subdir('winsys/lima/drm')
  subdir('drivers/lima')
else
  driver_lima = declare_dependency()
endif
if with_gallium_zink
  subdir('drivers/zink')
else
  driver_zink = declare_dependency()
endif
if with_gallium_d3d12
  subdir('drivers/d3d12')
else
  driver_d3d12 = declare_dependency()
endif
if with_gallium_opencl
  # TODO: this isn't really clover specific, but ATM clover is the only
  # consumer
  subdir('targets/pipe-loader')

  subdir('frontends/clover')
  subdir('targets/opencl')
endif
if with_gallium_rusticl
  subdir('frontends/rusticl')
  subdir('targets/rusticl')
endif
if with_dri
  subdir('frontends/dri')
  subdir('targets/dri')
endif
if with_osmesa
  subdir('frontends/osmesa')
  subdir('targets/osmesa')
endif
if with_glx == 'xlib'
  subdir('winsys/sw/xlib')
  subdir('frontends/glx/xlib')
  subdir('targets/libgl-xlib')
endif
if with_gallium_vdpau
  subdir('frontends/vdpau')
  subdir('targets/vdpau')
endif
if with_gallium_omx != 'disabled'
  subdir('frontends/omx')
  subdir('targets/omx')
endif
if with_gallium_va
  subdir('frontends/va')
  subdir('targets/va')
endif
if with_gallium_xa
  subdir('frontends/xa')
  subdir('targets/xa')
endif
if with_platform_haiku
  subdir('frontends/hgl')
endif
if with_gallium_st_nine
  subdir('frontends/nine')
  subdir('targets/d3dadapter9')
endif
if with_gallium_st_d3d10umd
  subdir('frontends/d3d10umd')
  subdir('targets/d3d10umd')
endif
if with_platform_windows
  if with_opengl
    subdir('frontends/wgl')
    if with_gallium_d3d12
      subdir('winsys/d3d12/wgl')
    else
      winsys_d3d12_wgl = declare_dependency()
    endif
    subdir('targets/wgl')
    subdir('targets/libgl-gdi')
  endif
endif
if with_tests
  subdir('tests')
endif
if with_swrast_vk
  subdir('frontends/lavapipe')
  subdir('targets/lavapipe')
endif

if with_teflon
  subdir('frontends/teflon')
  subdir('targets/teflon')
endif
