#pragma once

#include "KeyboardShortcut.hpp"
#include <string>
#include <unordered_map>

namespace OpenRCT2::Input
{
    class KeyboardShortcuts
    {
    public:
        KeyboardShortcuts(const std::string& configFile);
        ~KeyboardShortcuts() = default;

        void Load();
        void Reset();
        void Save();

    private:
        const std::string _configFile;
        const std::unordered_map<uint16_t, KeyboardShortcut> _defaultShortcuts;
        std::unordered_map<uint16_t, KeyboardShortcut> _shortcuts;
    };
} // namespace OpenRCT2::Input
