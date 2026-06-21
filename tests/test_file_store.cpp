#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>

#include <pypilot_settings.hpp>

int main() {
    const char* path = "pypilot_settings_file_store_test.conf";
    std::remove(path);
    std::remove("pypilot_settings_file_store_test.conf.tmp");

    {
        std::ofstream seed(path);
        seed << "# keep this comment\n";
        seed << "malformed line\n";
        seed << "ap.mode = compass\n";
    }

    pypilot_settings::FileSettingsStore store(path);
    char out[64]{};

    assert(store.load("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "compass") == 0);
    assert(!store.load("servo.max_current", out, sizeof(out)));

    assert(store.save("servo.max_current", "15.0"));
    assert(store.load("servo.max_current", out, sizeof(out)));
    assert(std::strcmp(out, "15.0") == 0);

    assert(store.save("ap.mode", "gps"));
    assert(store.load("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "gps") == 0);

    assert(store.erase("ap.mode"));
    assert(!store.load("ap.mode", out, sizeof(out)));
    assert(store.load("servo.max_current", out, sizeof(out)));
    assert(std::strcmp(out, "15.0") == 0);

    assert(!store.erase("not.present"));

    std::remove(path);
    std::remove("pypilot_settings_file_store_test.conf.tmp");
    return 0;
}
