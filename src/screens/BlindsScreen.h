#pragma once

#include "models/Blinds.h"

// Builds the blinds display screen (table number, blinds, ante, time remaining). Stateless: no buttons/callbacks.
class BlindsScreen
{
public:
    void show(const Blinds &blinds, int tableNumber);
};
