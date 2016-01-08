# Sanctum

Research prototype for a SGX clone that protects against all software attacks,
including timing attacks, and requires minimal architectural extensions to a
RISC V processor.


## Building the Code

### One-Time Setup

```bash
./scripts/deps/all.sh
./build/run_gyp.sh
```

### Build

If the `.gyp` files have been changed, GYP has to be re-run.

```bash
./build/run_gyp.sh
```

```bash
./build/ninja -C out/Release
./build/ninja -C out/Debug
```


## Development

### Running Unit Tests

```bash
./out/Debug/monitor_tests
```

### Useful Ninja Parameters

Serialized build, for debugging gyp issues.

```bash
./build/ninja -C out/Debug -j1
```

Don't stop on first error, build as much as possible.

```bash
./build/ninja -C out/Debug -k0
```


## Copyright

This work is Copyright (c) 2015 Victor Costan, all rights reserved.

Pending paper publication, this will be available under the MIT license.
