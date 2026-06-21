#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>

#include <pypilot_settings_catalog.hpp>
#include <pypilot_settings_store.hpp>

namespace pypilot_settings {

static inline std::string settings_trim(std::string text) {
    const char* whitespace = " \t\r\n";
    const size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) return std::string();
    const size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

static inline bool settings_parse_assignment(const std::string& line, std::string& name, std::string& value) {
    std::string trimmed = settings_trim(line);
    if (trimmed.empty() || trimmed[0] == '#') return false;
    const size_t eq = trimmed.find('=');
    if (eq == std::string::npos || eq == 0) return false;
    name = settings_trim(trimmed.substr(0, eq));
    value = settings_trim(trimmed.substr(eq + 1));
    return !name.empty();
}

class FileSettingsStore final : public ISettingsStore {
public:
    explicit FileSettingsStore(const char* path) : path_(path ? path : "") {}

    bool load(const char* name, char* out, size_t out_size) override {
        if (!name || !out || out_size == 0) return false;
        std::ifstream input(path_);
        if (!input.good()) return false;
        std::string line;
        std::string key;
        std::string value;
        bool found = false;
        std::string latest;
        while (std::getline(input, line)) {
            if (settings_parse_assignment(line, key, value) && key == name) {
                latest = value;
                found = true;
            }
        }
        if (!found) return false;
        return settings_copy(out, out_size, latest.c_str());
    }

    bool save(const char* name, const char* value) override {
        if (!name || !value || path_.empty()) return false;
        std::vector<std::string> lines;
        read_lines(lines);

        bool replaced = false;
        std::string key;
        std::string parsed_value;
        for (std::string& line : lines) {
            if (settings_parse_assignment(line, key, parsed_value) && key == name) {
                line = std::string(name) + "=" + value;
                replaced = true;
            }
        }
        if (!replaced) {
            lines.push_back(std::string(name) + "=" + value);
        }
        return write_lines(lines);
    }

    bool erase(const char* name) override {
        if (!name || path_.empty()) return false;
        std::vector<std::string> lines;
        read_lines(lines);
        std::vector<std::string> kept;
        bool removed = false;
        std::string key;
        std::string value;
        for (const std::string& line : lines) {
            if (settings_parse_assignment(line, key, value) && key == name) {
                removed = true;
                continue;
            }
            kept.push_back(line);
        }
        if (!removed) return false;
        return write_lines(kept);
    }

private:
    void read_lines(std::vector<std::string>& lines) const {
        std::ifstream input(path_);
        std::string line;
        while (std::getline(input, line)) {
            lines.push_back(line);
        }
    }

    bool write_lines(const std::vector<std::string>& lines) const {
        const std::string tmp = path_ + ".tmp";
        {
            std::ofstream output(tmp, std::ios::trunc);
            if (!output.good()) return false;
            for (const std::string& line : lines) {
                output << line << '\n';
            }
        }
        if (std::rename(tmp.c_str(), path_.c_str()) != 0) {
            std::remove(tmp.c_str());
            return false;
        }
        return true;
    }

    std::string path_;
};

} // namespace pypilot_settings
