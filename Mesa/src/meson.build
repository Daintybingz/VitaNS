# Copyright © 2017-2024 Intel Corporation
# SPDX-License-Identifier: MIT

with_egl = true
with_glx = 'disabled'
with_osmesa = 'disabled'
with_dri = false
with_gallium = true
with_vulkan_drivers = []
with_tools = []
with_gallium_va = false
with_gallium_vdpau = false
with_gallium_omx = 'disabled'
with_opencl = false
with_tests = false 

subdir('compiler')

# Patch: Remove Android stub inclusion for Nintendo Switch
# if with_platform_android and with_android_stub
#   subdir('android_stub')
# endif

# Subdirs that should always be present and built
subdir('util')
subdir('c11')
subdir('mapi')
subdir('mesa')

if with_tests
  subdir('test')
endif

if with_egl
  subdir('egl')
endif

if with_glx != 'disabled'
  subdir('glx')
endif

if with_osmesa != 'disabled'
  subdir('osmesa')
endif

if with_dri
  subdir('dri')
endif

if with_gallium
  subdir('gallium')
endif

if with_vulkan_drivers.length() > 0
  subdir('vulkan')
endif

if with_tools.length() > 0
  subdir('tools')
endif

if with_gallium_va or with_gallium_vdpau or with_gallium_omx != 'disabled'
  subdir('frontends')
endif

if with_opencl
  subdir('clc')
endif

# Patch: Remove android_stub or mapper checks
# Prevent conditional logic that depends on Android in this src/meson.build
