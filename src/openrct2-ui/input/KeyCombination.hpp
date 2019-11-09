#pragma once

#include <openrct2/common.h>

namespace OpenRCT2::Input
{
    class KeyCombination
    {
    public:
        KeyCombination(const rct_string_id id, const uint16_t scanCode);
        ~KeyCombination() = default;

        rct_string_id GetId();
        uint16_t GetScanCode();
    private:
        const rct_string_id _id;
        const uint16_t _scanCode;
    };
} // namespace OpenRCT2::Input
