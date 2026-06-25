#pragma once

#include <pypilot_settings_catalog.hpp>
#include <pypilot_settings_store.hpp>
#include <pypilot_settings_memory_store.hpp>
#include <pypilot_settings_paths.hpp>
#include <pypilot_settings_version.hpp>
#include <pypilot_settings_save_policy.hpp>
#include <pypilot_settings_servo_serial.hpp>

#if !defined(ARDUINO)
#include <pypilot_settings_file_store.hpp>
#include <pypilot_settings_config_store.hpp>
#endif

#if defined(ARDUINO) && defined(ESP32)
#include <pypilot_settings_nvs_store.hpp>
#endif

namespace pypilot_settings {

static inline const char* setting_scope_name(SettingScope scope) {
    switch (scope) {
    case SettingScope::Runtime: return "runtime";
    case SettingScope::Profile: return "profile";
    case SettingScope::Calibration: return "calibration";
    case SettingScope::Factory: return "factory";
    }
    return "unknown";
}

class SettingsManager final {
public:
    SettingsManager(const SettingsCatalog& catalog, ISettingsStore& store)
        : catalog_(catalog), store_(store) {}

    bool load_value(const char* name, char* out, size_t out_size) {
        if (store_.load(name, out, out_size)) return true;
        const SettingDescriptor* descriptor = catalog_.find(name);
        if (!descriptor) return false;
        return settings_copy(out, out_size, descriptor->default_value);
    }

    bool save_value(const char* name, const char* value, char* error, size_t error_size) {
        if (!catalog_.validate(name, value, error, error_size)) return false;
        return store_.save(name, value);
    }

    bool reset_value(const char* name) {
        return store_.erase(name);
    }

    void reset_scope(SettingScope scope) {
        for (size_t i = 0; i < catalog_.count(); ++i) {
            const SettingDescriptor& descriptor = catalog_.at(i);
            if (descriptor.scope == scope && descriptor.persistent) {
                store_.erase(descriptor.name);
            }
        }
    }

    size_t count_scope(SettingScope scope) const {
        size_t count = 0;
        for (size_t i = 0; i < catalog_.count(); ++i) {
            if (catalog_.at(i).scope == scope) ++count;
        }
        return count;
    }

    bool validate_value(const char* name, const char* value, char* error, size_t error_size) const {
        return catalog_.validate(name, value, error, error_size);
    }

private:
    const SettingsCatalog& catalog_;
    ISettingsStore& store_;
};

} // namespace pypilot_settings
