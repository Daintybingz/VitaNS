# Copyright © 2017 Intel Corporation

# Minimal Mesa for Nintendo Switch - Software rendering only

# Define missing variables for Switch build
with_asm_arch = 'none'
with_sse41 = false
with_platform_windows = false
with_shared_glapi = false
prog_python = find_program('python3', required: false)
cpp_msvc_compat_args = []

# Define missing API generation variables
main_api_exec_c = files()
main_api_exec_decl_h = files()
main_api_save_h = files()
main_api_save_init_h = files()
main_api_beginend_init_h = files()
main_api_hw_select_init_h = files()
main_enums_c = files()
ir_expression_operation_h = files()
sha1_h = files()
main_unmarshal_table_c = files()
main_marshal_generated_c = files()

# Define missing dependencies
dep_vdpau = null_dep
idep_nir = null_dep
idep_vtn = null_dep
libglsl = []
libmesa_sse41 = []

# Define missing include directories
inc_include = include_directories('../../include')
inc_src = include_directories('..')
inc_mapi = include_directories('../mapi')
inc_mesa = include_directories('.')
inc_gallium = include_directories('../gallium')
inc_gallium_aux = include_directories('../gallium/auxiliary')

# Define missing variables from other files
gl_and_es_api_files = []
glapi_gen_depends = []
mesa_lex = files()
program_parse_tab = files()
main_marshal_generated_h = files()
main_dispatch_h = files()
main_remap_helper_h = files()
format_fallback_c = files()
get_hash_h = files()

# Minimal stub files for basic libraries
files_libmesa = files(
  # Simple stub implementation
  'stub_mesa.c',
)

# No generated files needed for stub

# Main Mesa library for Switch
libmesa = static_library(
  'mesa',
  files_libmesa,
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src],
  build_by_default : true,
)

# We'll use the libglapi_static directly, so we don't need to reference individual files

# GLAPI library - use the existing libglapi_static from src/mapi/glapi/
libglapi = declare_dependency(
  link_with : libglapi_static,
  include_directories : [inc_include, inc_mapi, inc_src],
)

# OpenGL ES 2.0 library
es2_glapi_mapi_tmp_h = custom_target(
  'es2_glapi_mapi_tmp.h',
  input : [files('../mapi/new/gen_gldispatch_mapi.py'), files('../mapi/glapi/gen/gl_and_es_API.xml')],
  output : 'es2_glapi_mapi_tmp.h',
  command : [prog_python, '@INPUT0@', 'glesv2', '@INPUT1@'],
  capture : true,
)

libgles2 = static_library(
  'GLESv2',
  files(
    '../mapi/entry.c',
  ),
  es2_glapi_mapi_tmp_h,
  include_directories : [inc_include, inc_mapi, inc_src],
  c_args : [
    c_msvc_compat_args,
    '-DMAPI_MODE_BRIDGE',
    '-DMAPI_ABI_HEADER="@0@"'.format(es2_glapi_mapi_tmp_h.full_path()),
  ],
  link_with : [libglapi_static],
  dependencies : [dep_thread, idep_mesautilc11],
  gnu_symbol_visibility : 'hidden',
  build_by_default : true,
)

