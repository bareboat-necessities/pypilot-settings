# pypilot-settings

Persistent/config settings support for the modular C++ pypilot port.

This module is intentionally separate from `pypilot-runtime`:

- `pypilot-runtime` owns typed runtime values, TCP protocol handling, watches, and runtime clients.
- `pypilot-settings` owns setting descriptors, validation, and storage backends.

The module is dependency-light and does not depend on `pypilot-runtime`.

## Implemented features

- CMake header-only library.
- Linux CI.
- Setting descriptors.
- Bool, number, string, and enum setting types.
- Setting scopes for runtime, profile, calibration, and factory settings.
- Validation for bool syntax, number ranges, string length, enum choices, unknown settings, and non-writable settings.
- In-memory settings store for tests and embedded use.
- File-backed settings store using a simple `name=value` format for Linux/native use.
- `SettingsManager` facade that validates values before saving and returns descriptor defaults when a setting is not stored.
- Basic example.
- Unit tests.

## File format

`FileSettingsStore` uses a simple text format:

```text
# comments are ignored
ap.mode=compass
servo.max_current=15.0
imu.locked=false
```

Rules:

- blank lines are ignored;
- lines beginning with `#` are ignored;
- malformed lines are ignored on read and preserved during save;
- repeated keys are allowed on read, with the last matching value returned;
- save updates an existing key or appends a new `name=value` line;
- erase removes all matching key lines.

## Layout

```text
src/
  pypilot_settings.hpp
  pypilot_settings_catalog.hpp
  pypilot_settings_store.hpp
  pypilot_settings_memory_store.hpp
  pypilot_settings_file_store.hpp
tests/
  test_settings_catalog.cpp
  test_memory_store.cpp
  test_file_store.cpp
  test_validation.cpp
examples/
  SettingsExample/SettingsExample.cpp
```

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Design constraints

This module may use a descriptor catalog because it is metadata/storage logic. It must not replace runtime's hardcoded typed dispatch. Runtime value updates should remain explicit and typed in `pypilot-runtime`.

Future work can add:

- ESP32 NVS store.
- profile/calibration helpers.
- runtime bridge that lets `pypilot-runtime` load/save persistent settings through this module.
