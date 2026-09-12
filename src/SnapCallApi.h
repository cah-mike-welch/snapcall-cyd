#ifndef SNAPCALL_API_H
#define SNAPCALL_API_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <vector>
#include "Models.h"

// ============================================================================
// SNAPCALL API
// ============================================================================

/**
 * SnapCallApi provides methods to interact with the SnapCall device API.
 */
class SnapCallApi
{
public:
    /**
     * Initialize the API (can be empty for now).
     */
    void begin();

    /**
     * Retrieve the list of clubs.
     *
     * @return A vector of Club objects, or empty vector on error.
     */
    std::vector<Club> getClubs();

    /**
     * Retrieve the current blind level for a specific club.
     *
     * @param clubId The ID of the club.
     * @return BlindLevel with current blind information, or empty BlindLevel on error.
     */
    BlindLevel getCurrentBlinds(int clubId);

private:
    /**
     * Make an HTTP GET request and return the raw response body.
     *
     * @param endpoint The API endpoint (relative to base URL).
     * @return The raw response body as a String, or empty string on error.
     */
    String httpGet(const String &endpoint);
};

#endif // SNAPCALL_API_H
