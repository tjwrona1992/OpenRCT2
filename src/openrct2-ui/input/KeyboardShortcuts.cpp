#include "KeyboardShortcuts.hpp"
#include <openrct2/core/Console.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace OpenRCT2::Input
{
    static std::unordered_map<uint16_t, KeyboardShortcut> gDefaultShortcuts = {

    };

    KeyboardShortcuts::KeyboardShortcuts(const std::string& configFile)
        : _configFile(configFile)
        , _shortcuts(gDefaultShortcuts)
    {
        // If a config file already exists, load it
        Load();
    }

    void KeyboardShortcuts::Load()
    {
        try
        {
            // TODO: Load shortcuts from file
        }
        catch (const std::exception& ex)
        {
            Console::WriteLine("Error loading keyboard shortcuts: %s", ex.what());
        }
    }

    void KeyboardShortcuts::Reset()
    {
        _shortcuts = GetDefaultShortcuts();
    }

    void KeyboardShortcuts::Save()
    {
        try
        {
            // TODO: Save shortcuts to file
        }
        catch (const std::exception& ex)
        {
            Console::WriteLine("Error saving keyboard shortcuts: %s", ex.what());
        }
    }

} // namespace OpenRCT2::Input
