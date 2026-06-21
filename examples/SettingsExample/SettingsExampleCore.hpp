#pragma once

#include <pypilot_settings.hpp>

#if defined(ARDUINO)
#include <Arduino.h>
static inline void settings_example_print(const char* text) {
    Serial.println(text);
}
#else
#include <stdio.h>
static inline void settings_example_print(const char* text) {
    printf("%s\n", text);
}
#endif

static inline int run_settings_example_once() {
    using namespace pypilot_settings;
    const char* modes[] = {"compass", "gps", "wind"};
    const SettingDescriptor descriptors[] = {
        {"ap.mode", SettingType::Enum, SettingScope::Runtime, true, true, "compass", 0.0, 0.0, 0, modes, 3},
        {"servo.max_current", SettingType::Number, SettingScope::Calibration, true, true, "15.0", 0.0, 50.0, 0, nullptr, 0},
        {"profile.name", SettingType::String, SettingScope::Profile, true, true, "default", 0.0, 0.0, 16, nullptr, 0}
    };

    SettingsCatalog catalog(descriptors, 3);
    MemorySettingsStore<8> store;
    SettingsManager manager(catalog, store);

    char error[128]{};
    if (!manager.save_value("ap.mode", "gps", error, sizeof(error))) {
        settings_example_print(error);
        return 1;
    }

    char value[64]{};
    manager.load_value("ap.mode", value, sizeof(value));

    char line[96]{};
#if defined(ARDUINO)
    snprintf(line, sizeof(line), "ap.mode=%s", value);
#else
    snprintf(line, sizeof(line), "ap.mode=%s", value);
#endif
    settings_example_print(line);
    return 0;
}
