project(
  'mesa',
  'c',
  'cpp',
  version : '24.1.1',
  meson_version : '>= 1.1.0'
)

# Mesa build configuration for Nintendo Switch
# Force regular static archives instead of thin archives
add_project_arguments('-D__SWITCH__', '-DSWITCH', '-D__aarch64__', '-D__ARM_64BIT_STATE=1', language: 'c')
add_project_arguments('-D__SWITCH__', '-DSWITCH', '-D__aarch64__', '-D__ARM_64BIT_STATE=1', language: 'cpp')

# Configure ar to create regular static archives (not thin archives)
ar = find_program('ar', required: true)
ar_args = ['--no-thin']
if ar.found()
  add_project_arguments('ARFLAGS=--no-thin', language: 'c')
  add_project_arguments('ARFLAGS=--no-thin', language: 'cpp')
endif

# Global variables for Switch build
platform_android = false
system_has_kms_drm = false
with_dri2 = false
with_dri3 = false
with_osmesa = false

# Disable dynamic linking
dep_dl = null_dep

# Enable only software rasterizer
gallium_drivers = ['swrast']

# Force static GLAPI
with_shared_glapi = false

# ... existing code ...

