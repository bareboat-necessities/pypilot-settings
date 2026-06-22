#pragma once

#if !defined(ARDUINO)

#include <cstddef>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>

#include <pypilot_settings_catalog.hpp>
#include <pypilot_settings_file_store.hpp>
#include <pypilot_settings_store.hpp>

namespace pypilot_settings {

static inline bool pypilot_config_profile_section(const std::string& line, std::string& profile) {
    const std::string trimmed = settings_trim(line);
    static const char* prefix = "[profile=\"";
    static const char* suffix = "\"]";
    const size_t prefix_len = strlen(prefix);
    const size_t suffix_len = strlen(suffix);
    if (trimmed.size() < prefix_len + suffix_len) return false;
    if (trimmed.compare(0, prefix_len, prefix) != 0) return false;
    if (trimmed.compare(trimmed.size() - suffix_len, suffix_len, suffix) != 0) return false;
    profile = trimmed.substr(prefix_len, trimmed.size() - prefix_len - suffix_len);
    return true;
}

class PypilotConfigStore final : public ISettingsStore {
public:
    explicit PypilotConfigStore(const char* path) : path_(path ? path : "") {}

    bool load(const char* name, char* out, size_t out_size) override {
        return load_global(name, out, out_size);
    }

    bool save(const char* name, const char* value) override {
        return save_global(name, value);
    }

    bool erase(const char* name) override {
        return erase_global(name);
    }

    bool load_global(const char* name, char* out, size_t out_size) const {
        return load_from_section(nullptr, name, out, out_size);
    }

    bool save_global(const char* name, const char* value) const {
        return save_to_section(nullptr, name, value);
    }

    bool erase_global(const char* name) const {
        return erase_from_section(nullptr, name);
    }

    bool load_profile(const char* profile, const char* name, char* out, size_t out_size) const {
        return load_from_section(profile && *profile ? profile : "default", name, out, out_size);
    }

    bool save_profile(const char* profile, const char* name, const char* value) const {
        return save_to_section(profile && *profile ? profile : "default", name, value);
    }

    bool erase_profile(const char* profile, const char* name) const {
        return erase_from_section(profile && *profile ? profile : "default", name);
    }

    bool recover_backup() const {
        const std::string backup = backup_path();
        std::ifstream input(backup);
        if (!input.good()) return false;
        input.close();
        std::remove(path_.c_str());
        return std::rename(backup.c_str(), path_.c_str()) == 0;
    }

private:
    void read_lines(std::vector<std::string>& lines) const {
        std::ifstream input(path_);
        std::string line;
        while (std::getline(input, line)) lines.push_back(line);
    }

    bool write_lines(const std::vector<std::string>& lines) const {
        if (path_.empty()) return false;
        const std::string tmp = path_ + ".tmp";
        const std::string backup = backup_path();
        {
            std::ofstream output(tmp, std::ios::trunc);
            if (!output.good()) return false;
            for (const std::string& line : lines) output << line << '\n';
            output.flush();
            if (!output.good()) {
                std::remove(tmp.c_str());
                return false;
            }
        }

        std::remove(backup.c_str());
        bool had_existing = false;
        {
            std::ifstream existing(path_);
            had_existing = existing.good();
        }
        if (had_existing && std::rename(path_.c_str(), backup.c_str()) != 0) {
            std::remove(tmp.c_str());
            return false;
        }
        if (std::rename(tmp.c_str(), path_.c_str()) != 0) {
            std::remove(tmp.c_str());
            if (had_existing) std::rename(backup.c_str(), path_.c_str());
            return false;
        }
        return true;
    }

    bool section_is_target(const char* target_profile, const std::string& active_profile, bool in_profile) const {
        if (!target_profile) return !in_profile;
        return in_profile && active_profile == target_profile;
    }

    bool load_from_section(const char* target_profile, const char* name, char* out, size_t out_size) const {
        if (!name || !out || out_size == 0 || path_.empty()) return false;
        std::ifstream input(path_);
        if (!input.good()) return false;

        std::string line;
        std::string active_profile;
        bool in_profile = false;
        std::string key;
        std::string value;
        std::string profile;
        std::string latest;
        bool found = false;

        while (std::getline(input, line)) {
            if (pypilot_config_profile_section(line, profile)) {
                active_profile = profile;
                in_profile = true;
                continue;
            }
            if (!section_is_target(target_profile, active_profile, in_profile)) continue;
            if (settings_parse_assignment(line, key, value) && key == name) {
                latest = value;
                found = true;
            }
        }
        if (!found) return false;
        return settings_copy(out, out_size, latest.c_str());
    }

    bool save_to_section(const char* target_profile, const char* name, const char* value) const {
        if (!name || !value || path_.empty()) return false;
        std::vector<std::string> lines;
        read_lines(lines);

        std::string active_profile;
        std::string profile;
        bool in_profile = false;
        bool found_section = target_profile == nullptr;
        bool replaced = false;
        size_t first_profile_index = lines.size();
        size_t insert_index = lines.size();
        std::string key;
        std::string parsed_value;

        for (size_t i = 0; i < lines.size(); ++i) {
            if (pypilot_config_profile_section(lines[i], profile)) {
                if (!in_profile && first_profile_index == lines.size()) first_profile_index = i;
                if (target_profile && found_section && insert_index == lines.size()) insert_index = i;
                active_profile = profile;
                in_profile = true;
                if (target_profile && active_profile == target_profile) {
                    found_section = true;
                    insert_index = i + 1;
                }
                continue;
            }

            if (!section_is_target(target_profile, active_profile, in_profile)) continue;
            if (target_profile && found_section && insert_index == lines.size()) insert_index = i + 1;
            if (settings_parse_assignment(lines[i], key, parsed_value) && key == name) {
                lines[i] = std::string(name) + "=" + value;
                replaced = true;
            }
        }

        if (!replaced) {
            const std::string assignment = std::string(name) + "=" + value;
            if (!target_profile) {
                lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(first_profile_index), assignment);
            } else if (found_section) {
                lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(insert_index), assignment);
            } else {
                lines.push_back(std::string("[profile=\"") + target_profile + "\"]");
                lines.push_back(assignment);
            }
        }

        return write_lines(lines);
    }

    bool erase_from_section(const char* target_profile, const char* name) const {
        if (!name || path_.empty()) return false;
        std::vector<std::string> lines;
        read_lines(lines);
        std::vector<std::string> kept;
        kept.reserve(lines.size());

        std::string active_profile;
        std::string profile;
        bool in_profile = false;
        bool removed = false;
        std::string key;
        std::string value;

        for (const std::string& line : lines) {
            if (pypilot_config_profile_section(line, profile)) {
                active_profile = profile;
                in_profile = true;
                kept.push_back(line);
                continue;
            }
            if (section_is_target(target_profile, active_profile, in_profile) &&
                settings_parse_assignment(line, key, value) && key == name) {
                removed = true;
                continue;
            }
            kept.push_back(line);
        }

        if (!removed) return false;
        return write_lines(kept);
    }

    std::string backup_path() const { return path_ + ".bak"; }

    std::string path_;
};

} // namespace pypilot_settings

#endif // !defined(ARDUINO)
