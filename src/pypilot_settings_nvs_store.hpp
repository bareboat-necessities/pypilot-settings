#pragma once

#include <pypilot_settings_catalog.hpp>
#include <pypilot_settings_store.hpp>

#if defined(ARDUINO) && defined(ESP32)
#include <Preferences.h>

namespace pypilot_settings {

class ArduinoNvsSettingsStore final : public ISettingsStore {
public:
    explicit ArduinoNvsSettingsStore(const char* ns = "pypilot") : namespace_(ns ? ns : "pypilot") {}

    bool begin(bool read_only = false) {
        return preferences_.begin(namespace_, read_only);
    }

    void end() {
        preferences_.end();
    }

    bool load(const char* name, char* out, size_t out_size) override {
        if (!name || !out || out_size == 0) return false;
        if (!preferences_.isKey(name)) return false;
        const String value = preferences_.getString(name, "");
        return settings_copy(out, out_size, value.c_str());
    }

    bool save(const char* name, const char* value) override {
        if (!name || !value) return false;
        return preferences_.putString(name, value) > 0;
    }

    bool erase(const char* name) override {
        if (!name || !preferences_.isKey(name)) return false;
        return preferences_.remove(name);
    }

private:
    const char* namespace_ = "pypilot";
    Preferences preferences_;
};

} // namespace pypilot_settings

#endif
