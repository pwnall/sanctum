{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tests',
      'type': 'none',
      'sources': [
        # 'monitor/sanctum-monitor.cc',
      ],
      'dependencies': [
        'bare/bare.gyp:bare_tests',
        'crypto/crypto.gyp:crypto_tests',
        'monitor/monitor.gyp:monitor_tests',
      ],
    },
  ],
}
