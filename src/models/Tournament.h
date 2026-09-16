#pragma once

#include <Arduino.h>
#include <vector>
#include "Table.h"

struct Tournament
{
    int id = 0;
    String name;
    std::vector<Table> tables;
};
