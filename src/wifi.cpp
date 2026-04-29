/**
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include "main.hpp"
#include "network.hpp"

#ifdef CONF_USE_ARDUINO_OTA
#include <ArduinoOTA.h>
#endif

static volatile bool ota_in_progress = false;

bool wifiIsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

bool otaIsInProgress()
{
    return ota_in_progress;
}

#ifdef CONF_USE_WIFI_MANAGER
#include <WiFiManager.h>
WiFiManager wifiManager;
#endif

#ifdef CONF_USE_ARDUINO_OTA
void otaInit()
{
    ota_in_progress = false;
    ArduinoOTA.setHostname(stationID);

#ifdef CONF_ARDUINO_OTA_PASSWORD
    ArduinoOTA.setPassword(CONF_ARDUINO_OTA_PASSWORD);
#endif

    ArduinoOTA.onStart([]() {
        ota_in_progress = true;
#ifdef CONF_MQTT
        mqttSetPaused(true);
#endif
        Serial.println("OTA update started");
    });

    ArduinoOTA.onEnd([]() {
        ota_in_progress = false;
#ifdef CONF_MQTT
        mqttSetPaused(false);
#endif
        Serial.println("OTA update finished");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA progress: %u%%\r", (progress * 100U) / total);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        ota_in_progress = false;
#ifdef CONF_MQTT
        mqttSetPaused(false);
#endif
        Serial.printf("OTA error[%u]\n", error);
    });

    ArduinoOTA.begin();
    Serial.printf("ArduinoOTA ready on host '%s'\n", stationID);
}

void otaLoop()
{
    ArduinoOTA.handle();
}
#endif

void wifiInit()
{
#ifdef CONF_USE_WIFI_MANAGER
    wifiManager.autoConnect("GAIA-A08");

#else
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (!wifiIsConnected())
    {
        // Check for the connection
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        Serial.println("Trying to connecting to WiFi..");
    }
#endif

    WiFi.setSleep(false);

    Serial.print("Connected to the WiFi network with IP address: ");

    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
}
