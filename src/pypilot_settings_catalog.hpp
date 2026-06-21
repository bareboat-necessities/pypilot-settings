#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace pypilot_settings {

enum class SettingType {
    Bool,
    Number,
    String,
    Enum
};

enum class SettingScope {
    Runtime,
    Profile,
    Calibration,
    Factory
};

struct SettingDescriptor {
    const char* name;
    SettingType type;
    SettingScope scope;
    bool persistent;
    bool writable;
    const char* default_value;
    double min_value;
    double max_value;
    size_t max_length;
    const char* const* choices;
    size_t choice_count;

    constexpr SettingDescriptor(const char* name_ = "",
                                SettingType type_ = SettingType::String,
                                SettingScope scope_ = SettingScope::Runtime,
                                bool persistent_ = false,
                                bool writable_ = true,
                                const char* default_value_ = "",
                                double min_value_ = 0.0,
                                double max_value_ = 0.0,
                                size_t max_length_ = 0,
                                const char* const* choices_ = nullptr,
                                size_t choice_count_ = 0)
        : name(name_),
          type(type_),
          scope(scope_),
          persistent(persistent_),
          writable(writable_),
          default_value(default_value_),
          min_value(min_value_),
          max_value(max_value_),
          max_length(max_length_),
          choices(choices_),
          choice_count(choice_count_) {}
};

static inline bool settings_copy(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) return false;
    if (!src) {
        dst[0] = '\0';
        return true;
    }
    size_t i = 0;
    for (; i + 1 < dst_size && src[i]; ++i) dst[i] = src[i];
    dst[i] = '\0';
    return src[i] == '\0';
}

static inline bool settings_strip_quotes(const char* text, char* out, size_t out_size) {
    if (!text || !out || out_size == 0) return false;
    const size_t len = strlen(text);
    if (len >= 2 && text[0] == '"' && text[len - 1] == '"') {
        const size_t n = len - 2;
        if (n + 1 > out_size) return false;
        memcpy(out, text + 1, n);
        out[n] = '\0';
        return true;
    }
    return settings_copy(out, out_size, text);
}

static inline bool settings_parse_bool(const char* text) {
    return text && (strcmp(text, "true") == 0 || strcmp(text, "false") == 0 || strcmp(text, "1") == 0 || strcmp(text, "0") == 0);
}

static inline bool settings_parse_number(const char* text, double& out) {
    if (!text || !*text) return false;
    char* end = nullptr;
    const double value = strtod(text, &end);
    if (!end || *end != '\0') return false;
    out = value;
    return true;
}

class SettingsCatalog final {
public:
    SettingsCatalog(const SettingDescriptor* descriptors, size_t count)
        : descriptors_(descriptors), count_(count) {}

    const SettingDescriptor* find(const char* name) const {
        if (!name) return nullptr;
        for (size_t i = 0; i < count_; ++i) {
            if (descriptors_[i].name && strcmp(descriptors_[i].name, name) == 0) {
                return &descriptors_[i];
            }
        }
        return nullptr;
    }

    bool validate(const char* name, const char* value, char* error, size_t error_size) const {
        const SettingDescriptor* descriptor = find(name);
        if (!descriptor) {
            snprintf(error, error_size, "unknown setting %s", name ? name : "");
            return false;
        }
        if (!descriptor->writable) {
            snprintf(error, error_size, "%s is not writable", name ? name : "");
            return false;
        }
        return validate_value(*descriptor, value, error, error_size);
    }

    bool validate_value(const SettingDescriptor& descriptor, const char* value, char* error, size_t error_size) const {
        if (!value) {
            snprintf(error, error_size, "%s has null value", descriptor.name);
            return false;
        }
        switch (descriptor.type) {
        case SettingType::Bool:
            if (!settings_parse_bool(value)) {
                snprintf(error, error_size, "%s expects bool", descriptor.name);
                return false;
            }
            return true;
        case SettingType::Number: {
            double parsed = 0.0;
            if (!settings_parse_number(value, parsed)) {
                snprintf(error, error_size, "%s expects number", descriptor.name);
                return false;
            }
            if (parsed < descriptor.min_value || parsed > descriptor.max_value) {
                snprintf(error, error_size, "%s out of range", descriptor.name);
                return false;
            }
            return true;
        }
        case SettingType::String: {
            char stripped[256]{};
            if (!settings_strip_quotes(value, stripped, sizeof(stripped))) {
                snprintf(error, error_size, "%s string too long", descriptor.name);
                return false;
            }
            if (descriptor.max_length > 0 && strlen(stripped) > descriptor.max_length) {
                snprintf(error, error_size, "%s string too long", descriptor.name);
                return false;
            }
            return true;
        }
        case SettingType::Enum: {
            char stripped[128]{};
            if (!settings_strip_quotes(value, stripped, sizeof(stripped))) {
                snprintf(error, error_size, "%s enum value too long", descriptor.name);
                return false;
            }
            for (size_t i = 0; i < descriptor.choice_count; ++i) {
                if (descriptor.choices[i] && strcmp(descriptor.choices[i], stripped) == 0) return true;
            }
            snprintf(error, error_size, "%s invalid choice", descriptor.name);
            return false;
        }
        }
        snprintf(error, error_size, "%s invalid type", descriptor.name);
        return false;
    }

    size_t count() const { return count_; }
    const SettingDescriptor& at(size_t index) const { return descriptors_[index]; }

private:
    const SettingDescriptor* descriptors_ = nullptr;
    size_t count_ = 0;
};

} // namespace pypilot_settings
