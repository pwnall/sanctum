{
  'variables': {
    'clang': 'build/llvm-build/Release+Debug+Asserts/bin',
    'library': 'static_library',
    'mac_sdk': '10.11',
    'mac_deployment_target': '10.7',
  },

  'make_global_settings': [
    ['CC', '<(clang)/clang'],
    ['CXX', '<(clang)/clang++'],
    ['LINK', '<(clang)/clang++'],
  ],
  'target_defaults': {
    'cflags': [
      '-Wall', '-Wextra', '-Werror',
      '-Wsign-compare',  # See http://llvm.org/bugs/show_bug.cgi?id=10448
      '-fcolor-diagnostics',
      '-fstrict-aliasing',
      '-fno-threadsafe-statics',
      '-std=c11',
      '-Wheader-hygiene',
    ],
    'cflags_cc': [
      '-std=c++11',
    ],
    'xcode_settings': {
      'ALWAYS_SEARCH_USER_PATHS': 'NO',
      'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',   # -std=c++11
      'CLANG_CXX_LIBRARY': 'libc++',            # -stdlib=libc++
      'GCC_C_LANGUAGE_STANDARD': 'c11',         # -std=c11
      'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
      'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
      'GCC_STRICT_ALIASING': 'YES',             # -fstrict-aliasing
      'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
      'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES',    # -Werror
      'GCC_WARN_ABOUT_MISSING_NEWLINE': 'YES',  # -Wnewline-eof
      'SDKROOT': 'macosx<(mac_sdk)',
      'MACOSX_DEPLOYMENT_TARGET': '<(mac_deployment_target)',
    },
    'configurations': {
      'Debug': {
        'defines': ['DEBUG', '_DEBUG'],
        'cflags': ['-g', '-O0'],
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',  # gyp defaults to -Os
          'ONLY_ACTIVE_ARCH': 'YES',
        },
      },
      'Release': {
        'cflags': [
          '-O3',
          # Needed by --gc-symbols ldflag.
          '-ffunction-sections', '-fdata-sections',
        ],
        'ldflags': [
          '-Wl,-O1',
          '-Wl,--as-needed',
          '-Wl,--gc-sections',  # Remove unused symbols.
        ],
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '3',  # -O3
        },
      },
    },
    'default_configuration': 'Release',
    'conditions': [
      ['OS=="mac"', {
        'cflags!': [
          '-Os',  # gyp defaults to -Os on OSX
        ],
      }],
    ],
  },
}
