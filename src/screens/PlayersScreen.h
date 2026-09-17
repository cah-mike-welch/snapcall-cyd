#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <lvgl.h>
#include "models/Player.h"
#include "screens/NavBar.h"

// Builds the "Players" grid screen (2 columns x up to 5 rows) plus the Blinds/Players nav bar.
// Tapping an occupied seat reports the selected player; empty seats are not clickable.
class PlayersScreen
{
public:
    using SelectCallback = std::function<void(const Player &)>;

    void show(const std::vector<Player> &players, std::function<void()> onNavigateToBlinds, SelectCallback onSelect);

private:
    struct ButtonContext
    {
        PlayersScreen *screen;
        Player player;
    };

    static void buttonEventCb(lv_event_t *e);

    NavBar navBar_;
    std::vector<std::unique_ptr<ButtonContext>> buttonContexts_;
    SelectCallback onSelect_;
};
