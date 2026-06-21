#include <cassert>
#include <cstring>

#include <pypilot_settings.hpp>

class ZeroToOne final : public pypilot_settings::ISettingsMigrationStep {
public:
    int from_version() const override { return 0; }
    int to_version() const override { return 1; }
    bool migrate(pypilot_settings::ISettingsStore& store) override {
        return store.save("migrated", "true");
    }
};

int main() {
    using namespace pypilot_settings;
    MemorySettingsStore<8> store;
    ZeroToOne step;
    ISettingsMigrationStep* steps[] = {&step};
    SettingsMigrationManager migrations(steps, 1, 1);
    char out[64]{};

    assert(pypilot_settings_load_schema_version(store) == 0);
    assert(migrations.migrate(store));
    assert(pypilot_settings_load_schema_version(store) == 1);
    assert(store.load("migrated", out, sizeof(out)));
    assert(std::strcmp(out, "true") == 0);

    const RuntimeSettingPolicy policies[] = {
        {"ap.mode", SettingSavePolicy::OnChange},
        {"servo.calibration", SettingSavePolicy::OnCalibrationComplete},
        {"profile.name", SettingSavePolicy::OnShutdown}
    };
    RuntimeSettingPolicyCatalog catalog(policies, 3);
    assert(catalog.policy_for("ap.mode") == SettingSavePolicy::OnChange);
    assert(setting_save_on_runtime_set(catalog.policy_for("ap.mode")));
    assert(setting_save_on_calibration_complete(catalog.policy_for("servo.calibration")));
    assert(setting_save_on_shutdown(catalog.policy_for("profile.name")));
    assert(catalog.policy_for("unknown") == SettingSavePolicy::Never);
    assert(std::strcmp(setting_save_policy_name(SettingSavePolicy::Manual), "manual") == 0);
    return 0;
}
