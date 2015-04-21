{
  'includes': [
    '../common.gypi',
  ],
  'variables': {
    'bare_common_sources': [
      'base_types.h',
      'bit_masking.h',
      'cpu_context.h',
      'memory.h',
      'page_tables.h',
      'phys_atomics.h',
      'phys_ptr.h',
      'traits.h',
    ],
  },
  'targets': [
    {
      # Used by code that runs on bare metal, e.g. bootrom and monitor.
      'target_name': 'bare',
      'type': '<(library)',
      'sources': [
        '<@(bare_common_sources)',
        'arch/riscv/base_types_arch.h',
        'arch/riscv/cpu_context_arch.h',
        'arch/riscv/memory_arch.h',
        'arch/riscv/page_tables_arch.h',
        'arch/riscv/phys_atomics_arch.h',
        'arch/riscv/phys_ptr_arch.h',
      ],
      'target_defaults': {
        'cflags_cc+': [
          '-fno-rtti',
          '-fno-exceptions',
        ],
        'xcode_settings': {
          'GCC_ENABLE_CPP_RTTI': 'NO',              # -fno-rtti
          'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',        # -fno-exceptions
        },
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
      'all_dependent_settings': {
        'cflags_cc': [
          '-fno-rtti',
          '-fno-exceptions',
        ],
        'xcode_settings': {
          'GCC_ENABLE_CPP_RTTI': 'NO',              # -fno-rtti
          'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',        # -fno-exceptions
        },
      },
    },
    {
      # Used by unit tests for code that runs on bare metal.
      'target_name': 'bare_testing',
      'type': '<(library)',
      'sources': [
        'arch/test/base_types_arch.h',
        'arch/test/cpu_context_arch.cc',
        'arch/test/cpu_context_arch.h',
        'arch/test/memory_arch.cc',
        'arch/test/memory_arch.h',
        'arch/test/page_tables_arch.h',
        'arch/test/phys_atomics_arch.h',
        'arch/test/phys_ptr_arch.cc',
        'arch/test/phys_ptr_arch.h',
        'arch/test/gtest_main.cc',
      ],
      'include_dirs': [
        '..',
        'arch/test',
      ],
      'dependencies': [
        '../deps/gtest.gyp:gtest',
        '../deps/libcxx.gyp:libc++',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
          'arch/test',
        ],
      },
    },
    {
      # Unit tests for the "bare" library.
      'target_name': 'bare_tests',
      'type': 'executable',
      'sources': [
        'base_types_test.cc',
        'bit_masking_test.cc',
        'cpu_context_test.cc',
        'memory_test.cc',
        'page_tables_test.cc',
        'phys_atomics_test.cc',
        'phys_ptr_test.cc',
        'traits_test.cc',
      ],
      'dependencies': [
        ':bare_testing',
        '../deps/gtest.gyp:gtest',
        '../deps/libcxx.gyp:libc++',
      ],
    },
  ],
}
