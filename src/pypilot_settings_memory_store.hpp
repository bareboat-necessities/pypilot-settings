#pragma once

#include <string.h>

#include <pypilot_settings_catalog.hpp>
#include <pypilot_settings_store.hpp>

namespace pypilot_settings {

template<size_t MaxEntries = 32, size_t NameSize = 64, size_t ValueSize = 128>
class MemorySettingsStore final : public ISettingsStore {
public:
    bool load(const char* name, char* out, size_t out_size) override {
        Entry* entry = find_entry(name);
        if (!entry || !entry->used) return false;
        return settings_copy(out, out_size, entry->value);
    }

    bool save(const char* name, const char* value) override {
        if (!name || !value) return false;
        Entry* entry = find_entry(name);
        if (!entry) entry = free_entry();
        if (!entry) return false;
        if (!settings_copy(entry->name, sizeof(entry->name), name)) return false;
        if (!settings_copy(entry->value, sizeof(entry->value), value)) return false;
        entry->used = true;
        return true;
    }

    bool erase(const char* name) override {
        Entry* entry = find_entry(name);
        if (!entry) return false;
        entry->used = false;
        entry->name[0] = '\0';
        entry->value[0] = '\0';
        return true;
    }

    size_t count() const {
        size_t used = 0;
        for (size_t i = 0; i < MaxEntries; ++i) if (entries_[i].used) ++used;
        return used;
    }

private:
    struct Entry {
        bool used = false;
        char name[NameSize]{};
        char value[ValueSize]{};
    };

    Entry* find_entry(const char* name) {
        if (!name) return nullptr;
        for (size_t i = 0; i < MaxEntries; ++i) {
            if (entries_[i].used && strcmp(entries_[i].name, name) == 0) return &entries_[i];
        }
        return nullptr;
    }

    Entry* free_entry() {
        for (size_t i = 0; i < MaxEntries; ++i) {
            if (!entries_[i].used) return &entries_[i];
        }
        return nullptr;
    }

    Entry entries_[MaxEntries]{};
};

} // namespace pypilot_settings
