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
      'dependencies': [
        'bare/bare.gyp:bare',
      ],
    },
  ],
}
