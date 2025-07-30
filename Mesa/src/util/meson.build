# Copyright © 2017 Intel Corporation
# Minimal Mesa util for Nintendo Switch - Software rendering only

# Essential include directories
inc_util = [inc_include, include_directories('..')]

# Disable all unnecessary features for Switch
with_amd_vk = false
with_tests = false
with_platform_android = false
with_platform_haiku = false
with_perfetto = false
with_gpuvis = false
with_shader_cache = false
use_xmlconfig = false

# Define all dependencies as null for minimal build
prog_python = find_program('python3', required: false)
dep_zlib = null_dep
dep_valgrind = null_dep
dep_zstd = null_dep
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

# Essential utility files for software rendering
files_mesa_util = files(
  'anon_file.h',
  'anon_file.c',
  'bigmath.h',
  'bitscan.c',
  'bitscan.h',
  'bitset.h',
  'blend.h',
  'blob.c',
  'blob.h',
  'box.h',
  'build_id.c',
  'build_id.h',
  'compiler.h',
  'compress.c',
  'compress.h',
  'crc32.c',
  'crc32.h',
  'double.c',
  'double.h',
  'enum_operators.h',
  'fast_idiv_by_const.c',
  'fast_idiv_by_const.h',
  'format_r11g11b10f.h',
  'format_rgb9e5.h',
  'format_srgb.h',
  'futex.c',
  'futex.h',
  'glheader.h',
  'half_float.c',
  'half_float.h',
  'hash_table.c',
  'hash_table.h',
  'hex.h',
  'u_idalloc.c',
  'u_idalloc.h',
  'list.h',
  'log.c',
  'macros.h',
  'memstream.c',
  'memstream.h',
  'mesa-sha1.c',
  'mesa-sha1.h',
  'mesa-blake3.c',
  'mesa-blake3.h',
  'os_time.c',
  'os_time.h',
  'os_file.c',
  'os_memory_fd.c',
  'os_misc.c',
  'os_misc.h',
  'os_socket.c',
  'os_socket.h',
  'ptralloc.h',
  'u_process.c',
  'u_process.h',
  'u_qsort.cpp',
  'u_qsort.h',
  'rwlock.c',
  'rwlock.h',
  'sha1/sha1.c',
  'sha1/sha1.h',
  'ralloc.c',
  'ralloc.h',
  'rand_xor.c',
  'rand_xor.h',
  'rb_tree.c',
  'rb_tree.h',
  'register_allocate.c',
  'register_allocate.h',
  'rgtc.c',
  'rgtc.h',
  'rounding.h',
  'set.c',
  'set.h',
  'simple_mtx.c',
  'simple_mtx.h',
  'slab.c',
  'slab.h',
  'softfloat.c',
  'softfloat.h',
  'sparse_array.c',
  'sparse_array.h',
  'string_buffer.c',
  'string_buffer.h',
  'strndup.h',
  'strtod.c',
  'strtod.h',
  'timespec.h',
  'u_atomic.c',
  'u_atomic.h',
  'u_call_once.c',
  'u_call_once.h',
  'u_dl.c',
  'u_dl.h',
  'u_dynarray.c',
  'u_dynarray.h',
  'u_endian.h',
  'u_hash_table.c',
  'u_hash_table.h',
  'u_pointer.h',
  'u_queue.c',
  'u_queue.h',
  'u_string.h',
  'u_thread.c',
  'u_thread.h',
  'u_vector.c',
  'u_vector.h',
  'u_math.c',
  'u_math.h',
  'u_memset.h',
  'u_mm.c',
  'u_mm.h',
  'u_pack_color.h',
  'u_debug.c',
  'u_debug.h',
  'u_debug_memory.c',
  'u_cpu_detect.c',
  'u_cpu_detect.h',
  'u_printf.c',
  'u_printf.h',
  'u_worklist.c',
  'u_worklist.h',
  'vma.c',
  'vma.h',
  'xxhash.h',
)

# Minimal debug stack for non-Android platforms
files_debug_stack = files(
  'dbghelp.h',
  'u_debug_stack.c',
  'u_debug_symbol.c',
  'u_debug_symbol.h',
)

# Dependencies for minimal build
deps_for_libmesa_util = [
  dep_zlib,
  dep_clock,
  dep_thread,
  dep_atomic,
  dep_m,
  dep_valgrind,
  dep_zstd,
  dep_dl,
  dep_unwind,
  dep_futex,
]

# Add blake3 if available
subdir('blake3')
deps_for_libmesa_util += idep_blake3

# Simple format_srgb target (no Python generation)
format_srgb = custom_target(
  'format_srgb',
  input : ['format_srgb.py'],
  output : 'format_srgb.c',
  command : [prog_python, '@INPUT0@'],
  capture : true,
)

# Add format subdirectory for essential format support
subdir('format')
files_mesa_util += files_mesa_format

# Main utility library
_libmesa_util = static_library(
  'mesa_util',
  [files_mesa_util, files_debug_stack, format_srgb],
  include_directories : [inc_util, include_directories('format')],
  dependencies : deps_for_libmesa_util,
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : true
)

idep_mesautil = declare_dependency(
  link_with : _libmesa_util,
  include_directories : [inc_util],
  dependencies : deps_for_libmesa_util,
)

# Minimal XML config (disabled for Switch)
idep_xmlconfig = declare_dependency(
  dependencies : [],
  link_with : [],
)

# Essential tools
files_xxd = files('xxd.py')
glsl2spirv = files('glsl2spirv.py')

# Minimal parson library
_libparson = static_library(
  'parson',
  ['parson.c', 'parson.h'],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

idep_parson = declare_dependency(
  link_with :  _libparson,
)
