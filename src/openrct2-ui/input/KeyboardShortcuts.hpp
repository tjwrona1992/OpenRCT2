#pragma once

#include "KeyboardShortcut.hpp"
#include <string>
#include <vector>

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
        std::vector<KeyboardShortcut> _shortcuts;
    };
} // namespace OpenRCT2::Input
