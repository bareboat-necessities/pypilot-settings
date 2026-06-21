#pragma once

#include <stddef.h>

namespace pypilot_settings {

class ISettingsStore {
public:
    virtual ~ISettingsStore() = default;
    virtual bool load(const char* name, char* out, size_t out_size) = 0;
    virtual bool save(const char* name, const char* value) = 0;
    virtual bool erase(const char* name) = 0;
};

} // namespace pypilot_settings
