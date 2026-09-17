#include "ApiClient.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <string.h>
#include "config/AppConfig.h"

ApiResult ApiClient::performRequest(const char *method, const char *url, JsonDocument &doc) const
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return ApiResult::NoWifi;
    }

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(15);

    HTTPClient http;
    http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    ApiResult result = ApiResult::HttpError;

    Serial.printf("[API] %s %s\n", method, url);

    if (http.begin(client, url))
    {
        http.addHeader("Authorization", String("Bearer ") + AppConfig::kBearerToken);
        http.addHeader("Accept", "application/json");
        http.addHeader("User-Agent", "ESP32-CYD");

        int httpCode = (strcmp(method, "POST") == 0) ? http.POST("") : http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["success"] == true)
            {
                result = ApiResult::Success;
            }
            else
            {
                Serial.printf("[API] JSON parse failed for: %s\n", url);
                result = ApiResult::ParseError;
            }
        }
        else
        {
            Serial.printf("[API] %s failed, code: %d, url: %s\n", method, httpCode, url);
        }

        http.end();
    }

    return result;
}

ApiResult ApiClient::getClubs(std::vector<Club> &clubs)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/clubs", AppConfig::kApiBaseUrl);

    JsonDocument doc;
    ApiResult result = performRequest("GET", url, doc);

    if (result != ApiResult::Success)
    {
        return result;
    }

    clubs.clear();
    JsonArray clubsJson = doc["data"]["clubs"];
    for (JsonObject clubJson : clubsJson)
    {
        Club club;
        club.id = clubJson["id"] | 0;
        club.shortName = clubJson["short_name"] | "Unknown";
        club.name = clubJson["name"] | "Unknown";
        clubs.push_back(club);
    }

    return ApiResult::Success;
}

ApiResult ApiClient::getCurrentTournament(int clubId, Tournament &tournament)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/clubs/%d/current-tournament", AppConfig::kApiBaseUrl, clubId);

    JsonDocument doc;
    ApiResult result = performRequest("GET", url, doc);

    if (result != ApiResult::Success)
    {
        return result;
    }

    tournament.id = doc["data"]["tournament"]["id"] | 0;
    tournament.name = doc["data"]["tournament"]["name"] | "Current Tournament";

    tournament.tables.clear();
    JsonArray tablesJson = doc["data"]["tournament"]["tables"];
    for (JsonObject tableJson : tablesJson)
    {
        Table table;
        table.tournamentId = tournament.id;
        table.id = tableJson["id"] | 0;
        table.tableNumber = tableJson["table_number"] | 0;
        tournament.tables.push_back(table);
    }

    return ApiResult::Success;
}

ApiResult ApiClient::getCurrentBlinds(int clubId, Blinds &blinds)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/clubs/%d/current-blinds", AppConfig::kApiBaseUrl, clubId);

    JsonDocument doc;
    ApiResult result = performRequest("GET", url, doc);

    if (result != ApiResult::Success)
    {
        return result;
    }

    const char *sbStr = doc["data"]["small_blind"] | "0";
    const char *bbStr = doc["data"]["big_blind"] | "0";
    const char *bbaStr = doc["data"]["big_blind_ante"] | "0";
    const char *timeRemStr = doc["data"]["time_remaining"] | "00:00";

    blinds.smallBlind = sbStr;
    blinds.bigBlind = bbStr;
    blinds.bigBlindAnte = bbaStr;
    blinds.timeRemaining = timeRemStr;
    blinds.isRunning = doc["data"]["is_running"] | false;
    blinds.levelType = doc["data"]["level_type"] | "Play";

    return ApiResult::Success;
}

ApiResult ApiClient::getPlayers(int clubId, int tableId, std::vector<Player> &players)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/clubs/%d/tables/%d/players", AppConfig::kApiBaseUrl, clubId, tableId);

    JsonDocument doc;
    ApiResult result = performRequest("GET", url, doc);

    if (result != ApiResult::Success)
    {
        return result;
    }

    players.clear();
    JsonArray playersJson = doc["data"]["players"];
    for (JsonObject playerJson : playersJson)
    {
        Player player;
        player.id = playerJson["id"] | 0;
        player.firstName = playerJson["first_name"] | "";
        player.lastName = playerJson["last_name"] | "";
        player.seatNumber = playerJson["seat_number"] | 0;
        players.push_back(player);
    }

    return ApiResult::Success;
}

ApiResult ApiClient::eliminatePlayer(int clubId, int playerId)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/clubs/%d/players/%d/eliminate", AppConfig::kApiBaseUrl, clubId, playerId);

    JsonDocument doc;
    return performRequest("POST", url, doc);
}
