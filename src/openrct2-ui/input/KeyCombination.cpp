#include "KeyCombination.hpp"

namespace OpenRCT2::Input
{
    KeyCombination::KeyCombination(const rct_string_id id, const uint16_t scanCode)
        : _id(id)
        , _scanCode(scanCode)
    {
    }

    rct_string_id KeyCombination::GetId()
    {
        return _id;
    }

    uint16_t KeyCombination::GetScanCode()
    {
        return _scanCode;
    }
} // namespace OpenRCT2::Input
