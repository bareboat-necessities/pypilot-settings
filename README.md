# pypilot-settings

Persistent/config settings support for the modular C++ pypilot port.

This module is intentionally separate from `pypilot-runtime`:

- `pypilot-runtime` owns typed runtime values, TCP protocol handling, watches, and runtime clients.
- `pypilot-settings` owns setting descriptors, validation, storage backends, file naming conventions, migration helpers, and daemon save policy helpers.

The module is dependency-light and does not depend on `pypilot-runtime`.

## Implemented features

- CMake header-only library.
- Linux and ESP32-S3 example CI.
- Setting descriptors.
- Bool, number, string, and enum setting types.
- Setting scopes for runtime, profile, calibration, and factory settings.
- Profile/calibration helpers through `setting_scope_name()`, `SettingsManager::count_scope()`, and `SettingsManager::reset_scope()`.
- Standard pypilot path helpers for `~/.pypilot`, `pypilot.conf`, `pypilot.conf.bak`, `persist_fail`, profiles, and calibration files.
- Profile filename sanitizing.
- Calibration filename conventions for `RTIMULib.ini`, `servocalibration`, and `ruddercalibration`.
- Validation for bool syntax, number ranges, string length, enum choices, unknown settings, and non-writable settings.
- In-memory settings store for tests and embedded use.
- File-backed settings store using a simple `name=value` format for Linux/native use.
- Backup/rollback file writes using `.tmp` and `.bak` files.
- ESP32 NVS settings store through `ArduinoNvsSettingsStore` when compiling under Arduino ESP32.
- Schema version helpers and migration-step runner.
- Runtime daemon save-policy helpers for on-change, shutdown, calibration-complete, manual, and never-save behavior.
- `SettingsManager` facade that validates values before saving and returns descriptor defaults when a setting is not stored.
- Universal Linux/Arduino `SettingsExample`.
- Unit tests.

## Standard paths

The default pypilot-compatible config directory is:

```text
~/.pypilot/
```

It can be overridden with:

```text
PYPILOT_CONFIG_DIR
```

Common files are:

```text
~/.pypilot/pypilot.conf
~/.pypilot/pypilot.conf.bak
~/.pypilot/persist_fail
~/.pypilot/RTIMULib.ini
~/.pypilot/servocalibration
~/.pypilot/ruddercalibration
```

Profile files use sanitized names under:

```text
~/.pypilot/profiles/<profile>.conf
```

Only letters, numbers, `_`, `-`, and `.` are kept in profile filenames; other characters are converted to `_`.

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
- erase removes all matching key lines;
- writes create a `.tmp` file, move the previous file to `.bak`, then move `.tmp` into place;
- if the final move fails, the `.bak` file is restored.

## ESP32 NVS backend

`ArduinoNvsSettingsStore` is available only when compiling for Arduino ESP32:

```cpp
pypilot_settings::ArduinoNvsSettingsStore store("pypilot");
store.begin();
store.save("ap.mode", "compass");
store.end();
```

The public umbrella header avoids including the native file-store backend under Arduino builds.

## Runtime daemon save policy

`SettingSavePolicy` defines when a daemon should persist a setting:

```text
Never
OnChange
OnShutdown
OnCalibrationComplete
Manual
```

Recommended policy:

```text
ap.mode                 OnChange
ap.pilot                OnChange
servo.engaged           OnChange
profile.*               OnShutdown or Manual
imu/servo calibration   OnCalibrationComplete
live commands           Never
sensor telemetry        Never
```

The runtime bridge in `pypilot-runtime` is responsible for applying these policies to runtime values without moving protocol dispatch into this module.

## Layout

```text
src/
  pypilot_settings.hpp
  pypilot_settings_catalog.hpp
  pypilot_settings_store.hpp
  pypilot_settings_memory_store.hpp
  pypilot_settings_file_store.hpp
  pypilot_settings_nvs_store.hpp
  pypilot_settings_paths.hpp
  pypilot_settings_version.hpp
  pypilot_settings_save_policy.hpp
tests/
  test_settings_catalog.cpp
  test_memory_store.cpp
  test_file_store.cpp
  test_scope_helpers.cpp
  test_paths.cpp
  test_version_policy.cpp
  test_validation.cpp
examples/
  SettingsExample/SettingsExampleCore.hpp
  SettingsExample/SettingsExample.cpp
  SettingsExample/SettingsExample.ino
```

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Design constraints

This module may use a descriptor catalog because it is metadata/storage logic. It must not replace runtime's hardcoded typed dispatch. Runtime value updates should remain explicit and typed in `pypilot-runtime`.

`pypilot-runtime` consumes this module through its own optional settings bridge. `pypilot-settings` stays independent and does not include runtime headers.
