#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <lvgl.h>
#include "models/Tournament.h"
#include "models/Table.h"

// Builds the "<tournament> - Select Table" screen. Owns its per-button data and reports the selected table.
class TableSelectionScreen
{
public:
    using SelectCallback = std::function<void(const Table &)>;

    void show(const Tournament &tournament, SelectCallback onSelect);

private:
    struct ButtonContext
    {
        TableSelectionScreen *screen;
        Table table;
    };

    static void buttonEventCb(lv_event_t *e);

    std::vector<std::unique_ptr<ButtonContext>> buttonContexts_;
    SelectCallback onSelect_;
};
