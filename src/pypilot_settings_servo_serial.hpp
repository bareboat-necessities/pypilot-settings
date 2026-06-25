#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pypilot_settings_paths.hpp>

#if !defined(ARDUINO)
#include <fstream>
#include <string>
#endif

namespace pypilot_settings {

static constexpr const char* PYPILOT_SERVO_DEVICE_FILENAME = "servodevice";
static constexpr const char* PYPILOT_SERIAL_PORTS_FILENAME = "serial_ports";
static constexpr const char* PYPILOT_BLACKLIST_SERIAL_PORTS_FILENAME = "blacklist_serial_ports";

struct PypilotServoSerialCandidate {
    char path[160];
    int baud;
    bool preferred;

    PypilotServoSerialCandidate() : path{0}, baud(38400), preferred(false) {}
};

template<size_t MaxCandidates>
class PypilotServoSerialCandidateList final {
public:
    PypilotServoSerialCandidateList() : count_(0) {}

    void clear() { count_ = 0; }
    size_t count() const { return count_; }
    const PypilotServoSerialCandidate& at(size_t index) const { return candidates_[index]; }
    const PypilotServoSerialCandidate* data() const { return candidates_; }

    bool add(const char* path, int baud = 38400, bool preferred = false) {
        if (!path || !path[0] || count_ >= MaxCandidates) return false;
        const int normalized_baud = baud > 0 ? baud : 38400;
        for (size_t i = 0; i < count_; ++i) {
            if (strcmp(candidates_[i].path, path) == 0 && candidates_[i].baud == normalized_baud) return true;
        }
        if (!settings_copy(candidates_[count_].path, sizeof(candidates_[count_].path), path)) return false;
        candidates_[count_].baud = normalized_baud;
        candidates_[count_].preferred = preferred;
        ++count_;
        return true;
    }

private:
    size_t count_;
    PypilotServoSerialCandidate candidates_[MaxCandidates]{};
};

static inline bool pypilot_servo_device_file_path(const char* config_dir, char* out, size_t out_size) {
    return settings_append_path(out, out_size, config_dir, PYPILOT_SERVO_DEVICE_FILENAME);
}

static inline bool pypilot_serial_ports_file_path(const char* config_dir, char* out, size_t out_size) {
    return settings_append_path(out, out_size, config_dir, PYPILOT_SERIAL_PORTS_FILENAME);
}

static inline bool pypilot_blacklist_serial_ports_file_path(const char* config_dir, char* out, size_t out_size) {
    return settings_append_path(out, out_size, config_dir, PYPILOT_BLACKLIST_SERIAL_PORTS_FILENAME);
}

static inline bool pypilot_parse_servo_device_text(const char* text, char* path_out, size_t path_out_size, int& baud_out) {
    if (!text || !path_out || path_out_size == 0) return false;
    const char* first_quote = strchr(text, '"');
    if (!first_quote) return false;
    const char* second_quote = strchr(first_quote + 1, '"');
    if (!second_quote) return false;
    const char* comma = strchr(second_quote + 1, ',');
    if (!comma) return false;
    const size_t len = static_cast<size_t>(second_quote - first_quote - 1);
    if (len + 1 > path_out_size) return false;
    memcpy(path_out, first_quote + 1, len);
    path_out[len] = '\0';
    baud_out = atoi(comma + 1);
    if (baud_out <= 0) baud_out = 38400;
    return path_out[0] != '\0';
}

static inline bool pypilot_format_servo_device_text(const char* path, int baud, char* out, size_t out_size) {
    if (!path || !out || out_size == 0) return false;
    const int normalized_baud = baud > 0 ? baud : 38400;
    const int written = snprintf(out, out_size, "[\"%s\",%d]", path, normalized_baud);
    return written >= 0 && static_cast<size_t>(written) < out_size;
}

static inline bool pypilot_serial_ports_allows_any(const char* const* ports, size_t count) {
    if (!ports || count == 0) return true;
    return count == 1 && ports[0] && strcmp(ports[0], "any") == 0;
}

#if !defined(ARDUINO)
static inline bool pypilot_read_trimmed_lines(const char* path, PypilotServoSerialCandidateList<64>& out, bool preferred = false) {
    if (!path) return false;
    std::ifstream input(path);
    if (!input.good()) return false;
    std::string line;
    while (std::getline(input, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' || line.back() == '\t')) line.pop_back();
        size_t first = 0;
        while (first < line.size() && (line[first] == ' ' || line[first] == '\t')) ++first;
        if (first != 0) line.erase(0, first);
        if (!line.empty()) out.add(line.c_str(), 38400, preferred);
    }
    return true;
}

static inline bool pypilot_load_servo_device_file(const char* config_dir, PypilotServoSerialCandidateList<64>& out) {
    char path[256]{};
    if (!pypilot_servo_device_file_path(config_dir, path, sizeof(path))) return false;
    std::ifstream input(path);
    if (!input.good()) return false;
    std::string text;
    std::getline(input, text);
    char device[160]{};
    int baud = 38400;
    if (!pypilot_parse_servo_device_text(text.c_str(), device, sizeof(device), baud)) return false;
    return out.add(device, baud, true);
}

static inline bool pypilot_load_serial_ports_file(const char* config_dir, PypilotServoSerialCandidateList<64>& out) {
    char path[256]{};
    if (!pypilot_serial_ports_file_path(config_dir, path, sizeof(path))) return false;
    return pypilot_read_trimmed_lines(path, out, false);
}

static inline bool pypilot_load_blacklist_serial_ports_file(const char* config_dir, PypilotServoSerialCandidateList<64>& out) {
    char path[256]{};
    if (!pypilot_blacklist_serial_ports_file_path(config_dir, path, sizeof(path))) return false;
    return pypilot_read_trimmed_lines(path, out, false);
}

static inline bool pypilot_save_servo_device_file(const char* config_dir, const char* device, int baud) {
    char path[256]{};
    if (!pypilot_servo_device_file_path(config_dir, path, sizeof(path))) return false;
    char value[224]{};
    if (!pypilot_format_servo_device_text(device, baud, value, sizeof(value))) return false;
    std::ofstream output(path, std::ios::trunc);
    if (!output.good()) return false;
    output << value << "\n";
    return true;
}
#endif

} // namespace pypilot_settings
