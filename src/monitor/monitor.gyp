{
  'includes': [
    '../common.gypi',
  ],
  'variables': {
    'monitor_sources': [
      'api_compile_check.cc',
      'cpu_core.cc',
      'cpu_core.h',
      'dram_regions.cc',
      'dram_regions.h',
      'public/api.h',
    ],
  },
  'targets': [
    {
      # The security monitor.
      'target_name': 'monitor',
      'type': '<(library)',
      'sources': [
        '<@(monitor_sources)',
      ],
      'target_defaults': {
        'cflags_cc+': [
        ],
        'xcode_settings': {
        },
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
    },
    {
      # Unit tests for the monitor.
      'target_name': 'monitor_tests',
      'type': 'executable',
      'sources': [
        '<@(monitor_sources)',
      ],
      'dependencies': [
        '../bare/bare.gyp:bare_testing',
        '../deps/gtest.gyp:gtest',
        '../deps/libcxx.gyp:libc++',
      ],
    },
  ],
}
