# Generate builtin_types.h
builtin_types_h = custom_target(
  'builtin_types.h',
  input : 'builtin_types_h.py',
  output : 'builtin_types.h',
  command : [prog_python, '@INPUT@', '@OUTPUT@'],
  capture : false,
  depend_files : files('builtin_types.py'),
)

files_libcompiler = files(
  'glsl_types.c',
  'shader_enums.c',
  'nir/nir_serialize.c',
  'nir/nir_serialize.h',
  'nir/nir_schedule.c',
  'nir/nir_opt_constant_folding.c',
  'nir/nir_opt_copy_prop_vars.c',
  'nir/nir_lower_vars_to_ssa.c',
  'nir/nir_lower_var_copies.c',
  'nir/nir_validate.c',
  'nir/nir_metadata.c',
  'nir/nir_print.c',
  'nir/nir.c'
)

inc_compiler = include_directories('.', 'nir')
inc_include = include_directories('../../include')

libcompiler = static_library(
  'mesa_compiler',
  [files_libcompiler, builtin_types_h],
  include_directories: [inc_include, inc_compiler],
  c_args: [
    '-Wno-missing-field-initializers',
    '-Wno-sign-compare',
    '-Wno-unused-parameter',
  ],
  dependencies: [],
  build_by_default: false,
)

libcompiler_dep = declare_dependency(
  link_with: libcompiler,
  include_directories: [inc_include, inc_compiler],
)

# Build NIR library
subdir('nir')

# Only keep NIR-based compiler parts. Avoid classic OpenGL GLSL IR and other legacy systems.
# Useful for Switch/GLES2-only Mesa builds (Gallium + swrast).

# We omit builtin_type_pack.c and related GLSL IR sources.
# If needed, extend this to include spirv, tgsi, or backend-specific pieces relevant to NIR workflows.
