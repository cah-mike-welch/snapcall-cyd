#pragma once

#include <functional>
#include <lvgl.h>

enum class NavTarget
{
    Blinds,
    Players
};

// Small "Blinds" / "Players" navigation buttons shown near the top of those two screens.
class NavBar
{
public:
    void show(NavTarget active, std::function<void()> onBlinds, std::function<void()> onPlayers);

private:
    static void blindsBtnEventCb(lv_event_t *e);
    static void playersBtnEventCb(lv_event_t *e);

    std::function<void()> onBlinds_;
    std::function<void()> onPlayers_;
};
