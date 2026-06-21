#pragma once

namespace pypilot_settings {

enum class SettingSavePolicy {
    Never,
    OnChange,
    OnShutdown,
    OnCalibrationComplete,
    Manual
};

static inline const char* setting_save_policy_name(SettingSavePolicy policy) {
    switch (policy) {
    case SettingSavePolicy::Never: return "never";
    case SettingSavePolicy::OnChange: return "on_change";
    case SettingSavePolicy::OnShutdown: return "on_shutdown";
    case SettingSavePolicy::OnCalibrationComplete: return "on_calibration_complete";
    case SettingSavePolicy::Manual: return "manual";
    }
    return "unknown";
}

static inline bool setting_save_on_runtime_set(SettingSavePolicy policy) {
    return policy == SettingSavePolicy::OnChange;
}

static inline bool setting_save_on_shutdown(SettingSavePolicy policy) {
    return policy == SettingSavePolicy::OnShutdown;
}

static inline bool setting_save_on_calibration_complete(SettingSavePolicy policy) {
    return policy == SettingSavePolicy::OnCalibrationComplete;
}

struct RuntimeSettingPolicy {
    const char* name = "";
    SettingSavePolicy policy = SettingSavePolicy::Never;
};

class RuntimeSettingPolicyCatalog final {
public:
    RuntimeSettingPolicyCatalog(const RuntimeSettingPolicy* policies, size_t count)
        : policies_(policies), count_(count) {}

    SettingSavePolicy policy_for(const char* name) const {
        if (!name) return SettingSavePolicy::Never;
        for (size_t i = 0; i < count_; ++i) {
            if (policies_[i].name && strcmp(policies_[i].name, name) == 0) {
                return policies_[i].policy;
            }
        }
        return SettingSavePolicy::Never;
    }

private:
    const RuntimeSettingPolicy* policies_ = nullptr;
    size_t count_ = 0;
};

} // namespace pypilot_settings
