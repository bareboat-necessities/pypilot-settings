# pypilot-settings

Persistent/config settings support for the modular C++ pypilot port.

This module is intentionally separate from `pypilot-runtime`:

- `pypilot-runtime` owns typed runtime values, TCP protocol handling, watches, and runtime clients.
- `pypilot-settings` owns setting descriptors, validation, and storage backends.

The first phases are dependency-light and do not depend on `pypilot-runtime`.

## Phase 0

Implemented:

- CMake header-only library.
- Linux CI.
- In-memory settings store.
- Basic example.
- Unit tests.

## Phase 1

Implemented:

- Setting descriptors.
- Bool, number, string, and enum setting types.
- Setting scopes.
- Validation for bool syntax, number ranges, string length, enum choices, unknown settings.
- `SettingsManager` facade that validates values before saving and returns defaults when a setting is not stored.

## Layout

```text
src/
  pypilot_settings.hpp
  pypilot_settings_catalog.hpp
  pypilot_settings_store.hpp
  pypilot_settings_memory_store.hpp
tests/
  test_settings_catalog.cpp
  test_memory_store.cpp
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

Future phases can add:

- Linux file store.
- ESP32 NVS store.
- profile/calibration scopes.
- runtime bridge that lets `pypilot-runtime` load/save persistent settings through this module.
