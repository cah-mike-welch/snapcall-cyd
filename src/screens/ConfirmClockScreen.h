#pragma once

#include <functional>
#include <lvgl.h>

// Confirmation screen before starting or stopping the clock: a large Start/Stop button and a Cancel button.
class ConfirmClockScreen
{
public:
    void show(bool isRunning, std::function<void()> onConfirm, std::function<void()> onCancel);

private:
    static void confirmBtnEventCb(lv_event_t *e);
    static void cancelBtnEventCb(lv_event_t *e);

    std::function<void()> onConfirm_;
    std::function<void()> onCancel_;
};
