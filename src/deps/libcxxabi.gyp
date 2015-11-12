{
  'includes': [
    '../common.gypi',
  ],
  'targets': [
    {
      'target_name': 'libc++abi',
      'type': '<(library)',
      'dependencies=': [],
      'sources': [
        '../../deps/libcxxabi/src/abort_message.cpp',
        '../../deps/libcxxabi/src/cxa_aux_runtime.cpp',
        '../../deps/libcxxabi/src/cxa_default_handlers.cpp',
        '../../deps/libcxxabi/src/cxa_demangle.cpp',
        '../../deps/libcxxabi/src/cxa_exception.cpp',
        '../../deps/libcxxabi/src/cxa_exception_storage.cpp',
        '../../deps/libcxxabi/src/cxa_guard.cpp',
        '../../deps/libcxxabi/src/cxa_handlers.cpp',
        '../../deps/libcxxabi/src/cxa_new_delete.cpp',
        '../../deps/libcxxabi/src/cxa_personality.cpp',
        '../../deps/libcxxabi/src/cxa_thread_atexit.cpp',
        '../../deps/libcxxabi/src/cxa_unexpected.cpp',
        '../../deps/libcxxabi/src/cxa_vector.cpp',
        '../../deps/libcxxabi/src/cxa_virtual.cpp',
        '../../deps/libcxxabi/src/exception.cpp',
        '../../deps/libcxxabi/src/private_typeinfo.cpp',
        '../../deps/libcxxabi/src/stdexcept.cpp',
        '../../deps/libcxxabi/src/typeinfo.cpp',
      ],
      'cflags': [
        '-fPIC',
        '-fstrict-aliasing',
        '-Wno-unused-function',
      ],
      'cflags_cc': [
        '-nostdinc++',
        '-std=c++11',
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
        ],
        'conditions': [
          ['OS!="mac"', {
            'libraries': [
              '-lgcc',
              '-lgcc_s',
            ],
          }],
        ],
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '../../deps/libcxxabi/include',
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
