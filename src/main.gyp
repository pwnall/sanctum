{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'monitor',
      'type': '<(library)',
      'sources': [
        # 'monitor/sanctum-monitor.cc',
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
    },
    {
      'target_name': 'monitor_tests',
      'sources': [
        'monitor/gtest_main.cc',
        'monitor/phys_ptr_test.cc',
        'monitor/traits_test.cc',
      ],
      'type': 'executable',
      'dependencies': [
        'deps/libcxx.gyp:libc++',
        'deps/gtest.gyp:gtest',
      ],
    },
  ],
}

