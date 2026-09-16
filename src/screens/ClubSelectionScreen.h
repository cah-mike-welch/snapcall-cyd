#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <lvgl.h>
#include "models/Club.h"

// Builds the "Select a Club" screen. Owns its per-button data and reports the selected club.
class ClubSelectionScreen
{
public:
    using SelectCallback = std::function<void(const Club &)>;

    void show(const std::vector<Club> &clubs, SelectCallback onSelect);

private:
    struct ButtonContext
    {
        ClubSelectionScreen *screen;
        Club club;
    };

    static void buttonEventCb(lv_event_t *e);

    std::vector<std::unique_ptr<ButtonContext>> buttonContexts_;
    SelectCallback onSelect_;
};
