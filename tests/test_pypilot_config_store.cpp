#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include <pypilot_settings.hpp>

int main() {
    using namespace pypilot_settings;

    const char* path = "test_pypilot_config_store.conf";
    std::remove(path);
    std::remove("test_pypilot_config_store.conf.bak");
    {
        std::ofstream out(path, std::ios::trunc);
        out << "ap.mode=compass\n";
        out << "servo.max_current=15.0\n";
        out << "[profile=\"default\"]\n";
        out << "ap.pilot=basic\n";
        out << "servo.max_current=12.0\n";
        out << "[profile=\"heavy weather\"]\n";
        out << "ap.pilot=wind\n";
    }

    PypilotConfigStore store(path);
    char value[128]{};

    assert(store.load_global("ap.mode", value, sizeof(value)));
    assert(std::strcmp(value, "compass") == 0);
    assert(!store.load_global("ap.pilot", value, sizeof(value)));

    assert(store.load_profile("default", "ap.pilot", value, sizeof(value)));
    assert(std::strcmp(value, "basic") == 0);
    assert(store.load_profile("heavy weather", "ap.pilot", value, sizeof(value)));
    assert(std::strcmp(value, "wind") == 0);

    assert(store.save_profile("default", "ap.pilot", "absolute"));
    assert(store.load_profile("default", "ap.pilot", value, sizeof(value)));
    assert(std::strcmp(value, "absolute") == 0);

    assert(store.save_global("ap.mode", "wind"));
    assert(store.load_global("ap.mode", value, sizeof(value)));
    assert(std::strcmp(value, "wind") == 0);

    assert(store.save_profile("light air", "ap.pilot", "simple"));
    assert(store.load_profile("light air", "ap.pilot", value, sizeof(value)));
    assert(std::strcmp(value, "simple") == 0);

    assert(store.erase_profile("default", "servo.max_current"));
    assert(!store.load_profile("default", "servo.max_current", value, sizeof(value)));
    assert(store.load_global("servo.max_current", value, sizeof(value)));
    assert(std::strcmp(value, "15.0") == 0);

    std::remove(path);
    std::remove("test_pypilot_config_store.conf.bak");
    return 0;
}
