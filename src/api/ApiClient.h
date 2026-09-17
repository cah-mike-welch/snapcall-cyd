#pragma once

#include <vector>
#include <ArduinoJson.h>
#include "models/Club.h"
#include "models/Tournament.h"
#include "models/Blinds.h"
#include "models/Player.h"

enum class ApiResult
{
    Success,
    NoWifi,
    HttpError,
    ParseError
};

// Handles all HTTP requests and JSON parsing for the SnapCall device API.
class ApiClient
{
public:
    ApiResult getClubs(std::vector<Club> &clubs);
    ApiResult getCurrentTournament(int clubId, Tournament &tournament);
    ApiResult getCurrentBlinds(int clubId, Blinds &blinds);
    ApiResult getPlayers(int clubId, int tableId, std::vector<Player> &players);
    ApiResult eliminatePlayer(int clubId, int playerId);

private:
    ApiResult performRequest(const char *method, const char *url, JsonDocument &doc) const;
};
