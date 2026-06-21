#include <cassert>
#include <cstring>

#include <pypilot_settings.hpp>

int main() {
    using namespace pypilot_settings;
    const SettingDescriptor descriptors[] = {
        {"profile.name", SettingType::String, SettingScope::Profile, true, true, "default", 0.0, 0.0, 16, nullptr, 0},
        {"profile.gain", SettingType::Number, SettingScope::Profile, true, true, "1.0", 0.0, 10.0, 0, nullptr, 0},
        {"imu.heading_offset", SettingType::Number, SettingScope::Calibration, true, true, "0.0", -180.0, 180.0, 0, nullptr, 0},
        {"factory.serial", SettingType::String, SettingScope::Factory, true, false, "", 0.0, 0.0, 32, nullptr, 0}
    };

    SettingsCatalog catalog(descriptors, 4);
    MemorySettingsStore<8> store;
    SettingsManager manager(catalog, store);
    char error[128]{};
    char out[64]{};

    assert(std::strcmp(setting_scope_name(SettingScope::Profile), "profile") == 0);
    assert(std::strcmp(setting_scope_name(SettingScope::Calibration), "calibration") == 0);
    assert(manager.count_scope(SettingScope::Profile) == 2);
    assert(manager.count_scope(SettingScope::Calibration) == 1);

    assert(manager.save_value("profile.name", "weekend", error, sizeof(error)));
    assert(manager.save_value("profile.gain", "2.5", error, sizeof(error)));
    assert(manager.save_value("imu.heading_offset", "4.0", error, sizeof(error)));

    manager.reset_scope(SettingScope::Profile);
    assert(manager.load_value("profile.name", out, sizeof(out)));
    assert(std::strcmp(out, "default") == 0);
    assert(manager.load_value("profile.gain", out, sizeof(out)));
    assert(std::strcmp(out, "1.0") == 0);
    assert(manager.load_value("imu.heading_offset", out, sizeof(out)));
    assert(std::strcmp(out, "4.0") == 0);

    manager.reset_scope(SettingScope::Calibration);
    assert(manager.load_value("imu.heading_offset", out, sizeof(out)));
    assert(std::strcmp(out, "0.0") == 0);
    return 0;
}
