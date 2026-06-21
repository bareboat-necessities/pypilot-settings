#pragma once

#include <pypilot_settings_catalog.hpp>
#include <pypilot_settings_store.hpp>
#include <pypilot_settings_memory_store.hpp>

namespace pypilot_settings {

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

    bool validate_value(const char* name, const char* value, char* error, size_t error_size) const {
        return catalog_.validate(name, value, error, error_size);
    }

private:
    const SettingsCatalog& catalog_;
    ISettingsStore& store_;
};

} // namespace pypilot_settings
