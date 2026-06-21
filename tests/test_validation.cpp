#include <cassert>
#include <cstring>

#include <pypilot_settings.hpp>

int main() {
    using namespace pypilot_settings;
    const char* modes[] = {"compass", "gps", "wind"};
    const SettingDescriptor descriptors[] = {
        {"ap.mode", SettingType::Enum, SettingScope::Runtime, true, true, "compass", 0.0, 0.0, 0, modes, 3},
        {"servo.max_current", SettingType::Number, SettingScope::Calibration, true, true, "15.0", 0.0, 50.0, 0, nullptr, 0},
        {"imu.locked", SettingType::Bool, SettingScope::Calibration, true, true, "false", 0.0, 0.0, 0, nullptr, 0},
        {"profile.name", SettingType::String, SettingScope::Profile, true, true, "default", 0.0, 0.0, 16, nullptr, 0},
        {"factory.serial", SettingType::String, SettingScope::Factory, true, false, "", 0.0, 0.0, 32, nullptr, 0}
    };

    SettingsCatalog catalog(descriptors, 5);
    MemorySettingsStore<8> store;
    SettingsManager manager(catalog, store);
    char error[128]{};
    char out[64]{};

    assert(manager.validate_value("imu.locked", "true", error, sizeof(error)));
    assert(manager.validate_value("imu.locked", "0", error, sizeof(error)));
    assert(!manager.validate_value("imu.locked", "maybe", error, sizeof(error)));

    assert(manager.validate_value("servo.max_current", "25.5", error, sizeof(error)));
    assert(!manager.validate_value("servo.max_current", "-1", error, sizeof(error)));
    assert(!manager.validate_value("servo.max_current", "51", error, sizeof(error)));
    assert(!manager.validate_value("servo.max_current", "abc", error, sizeof(error)));

    assert(manager.validate_value("ap.mode", "gps", error, sizeof(error)));
    assert(manager.validate_value("ap.mode", "\"wind\"", error, sizeof(error)));
    assert(!manager.validate_value("ap.mode", "invalid", error, sizeof(error)));

    assert(manager.validate_value("profile.name", "weekend", error, sizeof(error)));
    assert(!manager.validate_value("profile.name", "this-name-is-too-long", error, sizeof(error)));
    assert(!manager.validate_value("missing", "x", error, sizeof(error)));
    assert(!manager.save_value("factory.serial", "123", error, sizeof(error)));

    assert(manager.load_value("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "compass") == 0);
    assert(manager.save_value("ap.mode", "gps", error, sizeof(error)));
    assert(manager.load_value("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "gps") == 0);
    assert(manager.reset_value("ap.mode"));
    assert(manager.load_value("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "compass") == 0);
    return 0;
}
