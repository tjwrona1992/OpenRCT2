#include "KeyboardShortcut.hpp"

namespace OpenRCT2::Input
{
    KeyboardShortcut::KeyboardShortcut(
        const uint16_t keyCombination, const rct_string_id id, const std::function<void()>& action)
        : _keyCombination(keyCombination)
        , _id(id)
        , _action(action)
    {
    }

    uint16_t KeyboardShortcut::GetKeyCombination()
    {
        return _keyCombination;
    }

    rct_string_id KeyboardShortcut::GetId()
    {
        return _id;
    }

    std::function<void()> KeyboardShortcut::GetAction()
    {
        return _action;
    }

    void KeyboardShortcut::Execute()
    {
        _action();
    }

} // namespace OpenRCT2::Input
