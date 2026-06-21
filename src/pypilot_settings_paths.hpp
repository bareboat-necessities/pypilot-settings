#pragma once

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <pypilot_settings_catalog.hpp>

namespace pypilot_settings {

static constexpr const char* PYPILOT_CONFIG_ENV = "PYPILOT_CONFIG_DIR";
static constexpr const char* PYPILOT_CONFIG_DIRNAME = ".pypilot";
static constexpr const char* PYPILOT_CONFIG_FILENAME = "pypilot.conf";
static constexpr const char* PYPILOT_CONFIG_BACKUP_SUFFIX = ".bak";
static constexpr const char* PYPILOT_PERSIST_FAIL_FILENAME = "persist_fail";
static constexpr const char* PYPILOT_PROFILE_DIRNAME = "profiles";
static constexpr const char* PYPILOT_CALIBRATION_IMU_FILENAME = "RTIMULib.ini";
static constexpr const char* PYPILOT_CALIBRATION_SERVO_FILENAME = "servocalibration";
static constexpr const char* PYPILOT_CALIBRATION_RUDDER_FILENAME = "ruddercalibration";

enum class CalibrationFileKind {
    Imu,
    Servo,
    Rudder
};

static inline bool settings_append_path(char* out, size_t out_size, const char* a, const char* b) {
    if (!out || out_size == 0 || !a || !b) return false;
    const size_t alen = strlen(a);
    const bool slash = alen > 0 && a[alen - 1] == '/';
    const int written = snprintf(out, out_size, slash ? "%s%s" : "%s/%s", a, b);
    return written >= 0 && static_cast<size_t>(written) < out_size;
}

static inline bool pypilot_default_config_dir(char* out, size_t out_size) {
    if (!out || out_size == 0) return false;
    const char* env = getenv(PYPILOT_CONFIG_ENV);
    if (env && *env) return settings_copy(out, out_size, env);
    const char* home = getenv("HOME");
    if (!home || !*home) home = "/";
    return settings_append_path(out, out_size, home, PYPILOT_CONFIG_DIRNAME);
}

static inline bool pypilot_config_file_path(const char* config_dir, char* out, size_t out_size) {
    return settings_append_path(out, out_size, config_dir, PYPILOT_CONFIG_FILENAME);
}

static inline bool pypilot_backup_file_path(const char* config_dir, char* out, size_t out_size) {
    char base[256]{};
    if (!pypilot_config_file_path(config_dir, base, sizeof(base))) return false;
    const int written = snprintf(out, out_size, "%s%s", base, PYPILOT_CONFIG_BACKUP_SUFFIX);
    return written >= 0 && static_cast<size_t>(written) < out_size;
}

static inline bool pypilot_persist_fail_path(const char* config_dir, char* out, size_t out_size) {
    return settings_append_path(out, out_size, config_dir, PYPILOT_PERSIST_FAIL_FILENAME);
}

static inline bool pypilot_sanitize_profile_name(const char* profile, char* out, size_t out_size) {
    if (!out || out_size == 0) return false;
    if (!profile || !*profile) profile = "default";
    size_t j = 0;
    for (size_t i = 0; profile[i] && j + 1 < out_size; ++i) {
        const unsigned char c = static_cast<unsigned char>(profile[i]);
        if (isalnum(c) || c == '_' || c == '-' || c == '.') out[j++] = static_cast<char>(c);
        else out[j++] = '_';
    }
    out[j] = '\0';
    return j > 0 && profile[j] == '\0';
}

static inline bool pypilot_profile_file_path(const char* config_dir, const char* profile, char* out, size_t out_size) {
    char profile_dir[256]{};
    char safe[96]{};
    char filename[128]{};
    if (!settings_append_path(profile_dir, sizeof(profile_dir), config_dir, PYPILOT_PROFILE_DIRNAME)) return false;
    if (!pypilot_sanitize_profile_name(profile, safe, sizeof(safe))) return false;
    const int written = snprintf(filename, sizeof(filename), "%s.conf", safe);
    if (written < 0 || static_cast<size_t>(written) >= sizeof(filename)) return false;
    return settings_append_path(out, out_size, profile_dir, filename);
}

static inline const char* pypilot_calibration_filename(CalibrationFileKind kind) {
    switch (kind) {
    case CalibrationFileKind::Imu: return PYPILOT_CALIBRATION_IMU_FILENAME;
    case CalibrationFileKind::Servo: return PYPILOT_CALIBRATION_SERVO_FILENAME;
    case CalibrationFileKind::Rudder: return PYPILOT_CALIBRATION_RUDDER_FILENAME;
    }
    return PYPILOT_CALIBRATION_IMU_FILENAME;
}

static inline bool pypilot_calibration_file_path(const char* config_dir, CalibrationFileKind kind, char* out, size_t out_size) {
    return settings_append_path(out, out_size, config_dir, pypilot_calibration_filename(kind));
}

} // namespace pypilot_settings
