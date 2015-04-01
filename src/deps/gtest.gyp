{
  'includes': [
    '../common.gypi',
  ],
  'targets': [
    {
      'target_name': 'gtest',
      'type': '<(library)',
      'sources': [
        '../../deps/googletest/src/gtest.cc',
        '../../deps/googletest/src/gtest-death-test.cc',
        '../../deps/googletest/src/gtest-filepath.cc',
        '../../deps/googletest/src/gtest-port.cc',
        '../../deps/googletest/src/gtest-printers.cc',
        '../../deps/googletest/src/gtest-test-part.cc',
        '../../deps/googletest/src/gtest-typed-test.cc',
      ],
      'include_dirs': [
        '../../deps/googletest/include',
        '../../deps/googletest',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../../deps/googletest/include',
        ]
      },
      'dependencies': [
        'libcxx.gyp:libc++',
      ],
    }
  ]
}
