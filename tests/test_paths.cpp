#include <cassert>
#include <cstring>

#include <pypilot_settings.hpp>

int main() {
    using namespace pypilot_settings;
    char path[256]{};
    char safe[64]{};

    assert(pypilot_config_file_path("/tmp/pypilot", path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/pypilot.conf") == 0);

    assert(pypilot_backup_file_path("/tmp/pypilot", path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/pypilot.conf.bak") == 0);

    assert(pypilot_persist_fail_path("/tmp/pypilot", path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/persist_fail") == 0);

    assert(pypilot_sanitize_profile_name("weekend sail", safe, sizeof(safe)));
    assert(std::strcmp(safe, "weekend_sail") == 0);

    assert(pypilot_profile_file_path("/tmp/pypilot", "weekend sail", path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/profiles/weekend_sail.conf") == 0);

    assert(pypilot_calibration_file_path("/tmp/pypilot", CalibrationFileKind::Imu, path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/RTIMULib.ini") == 0);

    assert(pypilot_calibration_file_path("/tmp/pypilot", CalibrationFileKind::Servo, path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/servocalibration") == 0);

    assert(pypilot_calibration_file_path("/tmp/pypilot", CalibrationFileKind::Rudder, path, sizeof(path)));
    assert(std::strcmp(path, "/tmp/pypilot/ruddercalibration") == 0);
    return 0;
}
