#pragma once

#include <functional>
#include <lvgl.h>
#include "models/Player.h"

// Confirmation screen: "Eliminate <First> <Last>?" with Eliminate/Cancel buttons.
class ConfirmEliminateScreen
{
public:
    void show(const Player &player, std::function<void()> onConfirm, std::function<void()> onCancel);

private:
    static void eliminateBtnEventCb(lv_event_t *e);
    static void cancelBtnEventCb(lv_event_t *e);

    std::function<void()> onConfirm_;
    std::function<void()> onCancel_;
};
