{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tests',
      'type': 'none',
      'dependencies': [
        'bare/bare.gyp:bare_tests',
        'crypto/crypto.gyp:crypto_tests',
        'monitor/monitor.gyp:monitor_tests',
      ],
    },
    {
      'target_name': 'libs',
      'type': 'none',
      'dependencies': [
        'bare/bare.gyp:bare',
        'crypto/crypto.gyp:crypto',
        #'monitor/monitor.gyp:monitor',
      ],
    }
  ],
}
