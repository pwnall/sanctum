{
  'includes': [
    '../common.gypi',
  ],
  'targets': [
    {
      'target_name': 'libc++',
      'type': '<(library)',
      'dependencies=': [
        'libcxxabi.gyp:libc++abi',
      ],
      'sources': [
        '../../deps/libcxx/src/algorithm.cpp',
        '../../deps/libcxx/src/bind.cpp',
        '../../deps/libcxx/src/chrono.cpp',
        '../../deps/libcxx/src/condition_variable.cpp',
        '../../deps/libcxx/src/debug.cpp',
        '../../deps/libcxx/src/exception.cpp',
        '../../deps/libcxx/src/future.cpp',
        '../../deps/libcxx/src/hash.cpp',
        '../../deps/libcxx/src/ios.cpp',
        '../../deps/libcxx/src/iostream.cpp',
        '../../deps/libcxx/src/locale.cpp',
        '../../deps/libcxx/src/memory.cpp',
        '../../deps/libcxx/src/mutex.cpp',
        '../../deps/libcxx/src/new.cpp',
        '../../deps/libcxx/src/optional.cpp',
        '../../deps/libcxx/src/random.cpp',
        '../../deps/libcxx/src/regex.cpp',
        '../../deps/libcxx/src/shared_mutex.cpp',
        '../../deps/libcxx/src/stdexcept.cpp',
        '../../deps/libcxx/src/string.cpp',
        '../../deps/libcxx/src/strstream.cpp',
        '../../deps/libcxx/src/system_error.cpp',
        '../../deps/libcxx/src/thread.cpp',
        '../../deps/libcxx/src/typeinfo.cpp',
        '../../deps/libcxx/src/utility.cpp',
        '../../deps/libcxx/src/valarray.cpp',
      ],
      'cflags': [
        '-fPIC',
        '-fstrict-aliasing',
        '-Wall',
        '-Wextra',
        '-Wshadow',
        '-Wconversion',
        '-Wnewline-eof',
        '-Wpadded',
        '-Wmissing-prototypes',
        '-Wstrict-aliasing=2',
        '-Wstrict-overflow=4',
      ],
      'cflags_cc': [
        '-nostdinc++',
      ],
      'include_dirs': [
        '../../deps/libcxx/include',
        '../../deps/libcxxabi/include',
      ],
      'ldflags': [
        '-nodefaultlibs',
      ],
      'link_settings': {
        'libraries': [
          '-lpthread',
          '-lc',
          '-lm',
        ],
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '../../deps/libcxx/include',
        ],
      },
      'all_dependent_settings': {
        'cflags_cc': [
          '-nostdinc++',
        ],
        'ldflags': [
          '-nodefaultlibs',
        ],
      },
    },
  ],
}
