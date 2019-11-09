#include "Key.hpp"

namespace OpenRCT2::Input
{
    Key::Key(const rct_string_id id, const uint16_t scanCode)
        : _id(id)
        , _scanCode(scanCode)
    {
    }

    rct_string_id Key::GetId()
    {
        return _id;
    }

    uint16_t Key::GetScanCode()
    {
        return _scanCode;
    }
} // namespace OpenRCT2::Input
