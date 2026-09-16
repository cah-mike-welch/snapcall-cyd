#include "ApiClient.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "config/AppConfig.h"

ApiResult ApiClient::performGet(const char *url, JsonDocument &doc) const
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

    Serial.printf("[API] GET %s\n", url);

    if (http.begin(client, url))
    {
        http.addHeader("Authorization", String("Bearer ") + AppConfig::kBearerToken);
        http.addHeader("Accept", "application/json");
        http.addHeader("User-Agent", "ESP32-CYD");

        int httpCode = http.GET();

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
            Serial.printf("[API] GET failed, code: %d, url: %s\n", httpCode, url);
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
    ApiResult result = performGet(url, doc);

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
    ApiResult result = performGet(url, doc);

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
    ApiResult result = performGet(url, doc);

    if (result != ApiResult::Success)
    {
        return result;
    }

    const char *sbStr = doc["data"]["small_blind"] | "0";
    const char *bbStr = doc["data"]["big_blind"] | "0";
    const char *bbaStr = doc["data"]["big_blind_ante"] | "0";
    const char *timeRemStr = doc["data"]["time_remaining"] | "00:00";

    blinds.smallBlind = atoi(sbStr);
    blinds.bigBlind = atoi(bbStr);
    blinds.bigBlindAnte = atoi(bbaStr);
    blinds.timeRemaining = timeRemStr;

    return ApiResult::Success;
}
