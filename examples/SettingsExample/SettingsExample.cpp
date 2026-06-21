#include <stdio.h>

#include <pypilot_settings.hpp>

int main() {
    using namespace pypilot_settings;
    const char* modes[] = {"compass", "gps", "wind"};
    const SettingDescriptor descriptors[] = {
        {"ap.mode", SettingType::Enum, SettingScope::Runtime, true, true, "compass", 0.0, 0.0, 0, modes, 3},
        {"servo.max_current", SettingType::Number, SettingScope::Calibration, true, true, "15.0", 0.0, 50.0, 0, nullptr, 0}
    };

    SettingsCatalog catalog(descriptors, 2);
    MemorySettingsStore<8> store;
    SettingsManager manager(catalog, store);

    char error[128]{};
    if (!manager.save_value("ap.mode", "gps", error, sizeof(error))) {
        printf("save failed: %s\n", error);
        return 1;
    }

    char value[64]{};
    manager.load_value("ap.mode", value, sizeof(value));
    printf("ap.mode=%s\n", value);
    return 0;
}
