#include <cassert>
#include <cstring>

#include <pypilot_settings.hpp>

int main() {
    pypilot_settings::MemorySettingsStore<2> store;
    char out[64]{};

    assert(!store.load("ap.mode", out, sizeof(out)));
    assert(store.save("ap.mode", "compass"));
    assert(store.load("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "compass") == 0);
    assert(store.count() == 1);

    assert(store.save("ap.mode", "gps"));
    assert(store.load("ap.mode", out, sizeof(out)));
    assert(std::strcmp(out, "gps") == 0);
    assert(store.count() == 1);

    assert(store.save("servo.max_current", "15.0"));
    assert(store.count() == 2);
    assert(!store.save("wind.source", "nmea"));

    assert(store.erase("ap.mode"));
    assert(!store.load("ap.mode", out, sizeof(out)));
    assert(store.count() == 1);
    return 0;
}
