#include <cassert>
#include <cstring>

#include <pypilot_settings.hpp>

int main() {
    using namespace pypilot_settings;
    const char* modes[] = {"compass", "gps", "wind"};
    const SettingDescriptor descriptors[] = {
        {"ap.mode", SettingType::Enum, SettingScope::Runtime, true, true, "compass", 0.0, 0.0, 0, modes, 3},
        {"servo.max_current", SettingType::Number, SettingScope::Calibration, true, true, "15.0", 0.0, 50.0, 0, nullptr, 0},
        {"imu.locked", SettingType::Bool, SettingScope::Calibration, true, true, "false", 0.0, 0.0, 0, nullptr, 0}
    };

    SettingsCatalog catalog(descriptors, 3);
    assert(catalog.count() == 3);
    assert(catalog.find("ap.mode") != nullptr);
    assert(catalog.find("not.present") == nullptr);
    assert(std::strcmp(catalog.find("servo.max_current")->default_value, "15.0") == 0);
    return 0;
}
