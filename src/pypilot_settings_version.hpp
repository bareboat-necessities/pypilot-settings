#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <pypilot_settings_store.hpp>
#include <pypilot_settings_catalog.hpp>

namespace pypilot_settings {

static constexpr int PYPILOT_SETTINGS_SCHEMA_VERSION = 1;
static constexpr const char* PYPILOT_SETTINGS_SCHEMA_KEY = "settings.schema_version";

class ISettingsMigrationStep {
public:
    virtual ~ISettingsMigrationStep() = default;
    virtual int from_version() const = 0;
    virtual int to_version() const = 0;
    virtual bool migrate(ISettingsStore& store) = 0;
};

static inline int pypilot_settings_load_schema_version(ISettingsStore& store) {
    char value[32]{};
    if (!store.load(PYPILOT_SETTINGS_SCHEMA_KEY, value, sizeof(value))) return 0;
    return atoi(value);
}

static inline bool pypilot_settings_save_schema_version(ISettingsStore& store, int version) {
    char value[32]{};
    snprintf(value, sizeof(value), "%d", version);
    return store.save(PYPILOT_SETTINGS_SCHEMA_KEY, value);
}

class SettingsMigrationManager final {
public:
    SettingsMigrationManager(ISettingsMigrationStep* const* steps, size_t count, int target_version = PYPILOT_SETTINGS_SCHEMA_VERSION)
        : steps_(steps), count_(count), target_version_(target_version) {}

    bool migrate(ISettingsStore& store) const {
        int current = pypilot_settings_load_schema_version(store);
        if (current == target_version_) return true;
        while (current < target_version_) {
            ISettingsMigrationStep* step = find_step(current);
            if (!step) return false;
            if (!step->migrate(store)) return false;
            current = step->to_version();
            if (!pypilot_settings_save_schema_version(store, current)) return false;
        }
        return current == target_version_;
    }

private:
    ISettingsMigrationStep* find_step(int from_version) const {
        for (size_t i = 0; i < count_; ++i) {
            if (steps_[i] && steps_[i]->from_version() == from_version) return steps_[i];
        }
        return nullptr;
    }

    ISettingsMigrationStep* const* steps_ = nullptr;
    size_t count_ = 0;
    int target_version_ = PYPILOT_SETTINGS_SCHEMA_VERSION;
};

} // namespace pypilot_settings
