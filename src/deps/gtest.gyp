{
  'includes': [
    '../common.gypi',
  ],
  'targets': [
    {
      'target_name': 'gtest',
      'type': '<(library)',
      'sources': [
        '../../deps/googletest/googletest/src/gtest.cc',
        '../../deps/googletest/googletest/src/gtest-death-test.cc',
        '../../deps/googletest/googletest/src/gtest-filepath.cc',
        '../../deps/googletest/googletest/src/gtest-port.cc',
        '../../deps/googletest/googletest/src/gtest-printers.cc',
        '../../deps/googletest/googletest/src/gtest-test-part.cc',
        '../../deps/googletest/googletest/src/gtest-typed-test.cc',
      ],
      'include_dirs': [
        '../../deps/googletest/googletest/include',
        '../../deps/googletest/googletest',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../../deps/googletest/googletest/include',
        ]
      },
      'dependencies': [
        'libcxx.gyp:libc++',
      ],
    }
  ]
}
