#pragma once

#include <openrct2/common.h>

namespace OpenRCT2::Input
{
    /*!
    \brief Class to represent individual key presses.

    This class maps the RCT localized string ID to the corresponding SDL scancode for each keypress.
    */
    class Key
    {
    public:
        /*!
        \brief Constructs an object containing information about a specific keypress.

        \param[in] id The RCT localized string ID associated with the keypress.
        \param[in] scancode The SDL scancode associated with the keypress.
        */
        Key(const rct_string_id id, const uint16_t scancode);

        /*!
        \brief Destructor.

        Use the compiler generated default destructor.
        */
        ~Key() = default;

        /*!
        \brief Gets the RCT localized string for the keypress.

        \return RCT localized string for the keypress.
        */
        std::string GetString() const;

    private:
        const rct_string_id _id;
        const uint16_t _scancode;
    };
} // namespace OpenRCT2::Input
