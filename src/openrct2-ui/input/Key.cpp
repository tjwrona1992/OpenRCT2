#include "Key.hpp"
#include <openrct2/localisation/Localisation.h>
#include <SDL.h>
#include <string>

namespace OpenRCT2::Input
{
    

    Key::Key(uint16_t scancode)
        , _scancode(scancode)
    {
    }

    std::string Key::GetString() const
    {
        return format_string(_id, nullptr);
    }

} // namespace OpenRCT2::Input
