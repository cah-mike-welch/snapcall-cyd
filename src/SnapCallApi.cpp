#include "SnapCallApi.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

static const char *TAG = "[API]";

void SnapCallApi::begin()
{
    // Initialization can be extended later if needed
}

String SnapCallApi::httpGet(const String &endpoint)
{
    String url = String(SNAPCALL_API_BASE_URL) + endpoint;

    Serial.printf("%s Making GET request to: %s\n", TAG, url.c_str());

    String payload = "";
    
    // Handle HTTPS with certificate validation disabled
    if (url.startsWith("https")) {
        WiFiClientSecure client;
        client.setInsecure();  // Skip SSL certificate validation
        
        HTTPClient http;
        http.begin(client, url);
        http.addHeader("Authorization", String("Bearer ") + SNAPCALL_API_KEY);
        http.addHeader("Content-Type", "application/json");
        
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            payload = http.getString();
            Serial.printf("%s HTTP %d - Success\n", TAG, httpCode);
        } else if (httpCode == 401) {
            Serial.printf("%s HTTP 401 - Unauthorized (check API key)\n", TAG);
            Serial.printf("%s Response: %s\n", TAG, http.getString().c_str());
        } else if (httpCode == 301 || httpCode == 302 || httpCode == 307 || httpCode == 308) {
            // Log redirect information
            String location = http.header("Location");
            Serial.printf("%s HTTP %d - Redirect to: %s\n", TAG, httpCode, location.c_str());
        } else {
            Serial.printf("%s HTTP Error: %d\n", TAG, httpCode);
        }
        
        http.end();
    } else {
        HTTPClient http;
        http.begin(url);
        http.addHeader("Authorization", String("Bearer ") + SNAPCALL_API_KEY);
        http.addHeader("Content-Type", "application/json");
        
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            payload = http.getString();
            Serial.printf("%s HTTP %d - Success\n", TAG, httpCode);
        } else {
            Serial.printf("%s HTTP Error: %d\n", TAG, httpCode);
        }
        
        http.end();
    }
    
    return payload;
}

std::vector<Club> SnapCallApi::getClubs()
{
    std::vector<Club> clubs;

    Serial.printf("%s GET /clubs\n", TAG);

    String payload = httpGet("/clubs");

    if (payload.isEmpty())
    {
        Serial.printf("%s Failed to get response\n", TAG);
        return clubs;
    }

    // Parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.printf("%s JSON parse error: %s\n", TAG, error.c_str());
        return clubs;
    }

    // Check for success flag
    bool success = doc["success"] | false;
    if (!success)
    {
        Serial.printf("%s API returned success=false\n", TAG);
        return clubs;
    }

    // Extract clubs array
    JsonArray clubsArray = doc["data"]["clubs"];
    if (!clubsArray.isNull())
    {
        for (JsonObject clubObj : clubsArray)
        {
            int id = clubObj["id"] | 0;
            String shortName = clubObj["short_name"] | "";
            String name = clubObj["name"] | "";

            clubs.emplace_back(id, shortName, name);
        }
    }

    Serial.printf("%s success=true\n", TAG);
    Serial.printf("%s Clubs loaded: %d\n", TAG, clubs.size());

    return clubs;
}

BlindLevel SnapCallApi::getCurrentBlinds(int clubId)
{
    BlindLevel blinds;

    String endpoint = String("/clubs/") + String(clubId) + String("/current-blinds");
    Serial.printf("%s GET %s\n", TAG, endpoint.c_str());

    String payload = httpGet(endpoint);

    if (payload.isEmpty())
    {
        Serial.printf("%s Failed to get response\n", TAG);
        return blinds;
    }

    // Parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.printf("%s JSON parse error: %s\n", TAG, error.c_str());
        return blinds;
    }

    // Check for success flag
    bool success = doc["success"] | false;
    if (!success)
    {
        Serial.printf("%s API returned success=false\n", TAG);
        return blinds;
    }

    // Extract blind values
    const char *sbStr = doc["data"]["small_blind"] | "";
    const char *bbStr = doc["data"]["big_blind"] | "";
    const char *bbaStr = doc["data"]["big_blind_ante"] | "";
    const char *timeStr = doc["data"]["time_remaining"] | "";

    blinds.smallBlind = atoi(sbStr);
    blinds.bigBlind = atoi(bbStr);
    blinds.bigBlindAnte = atoi(bbaStr);
    blinds.timeRemaining = String(timeStr);

    Serial.printf("%s Blinds: %d/%d ante %d\n", TAG, blinds.smallBlind, blinds.bigBlind, blinds.bigBlindAnte);
    Serial.printf("%s Time remaining: %s\n", TAG, blinds.timeRemaining.c_str());

    return blinds;
}
