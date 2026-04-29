
#ifndef _GAIA_NETWORK
#define _GAIA_NETWORK

#include <ArduinoJson.h>

void wifiInit();
bool wifiIsConnected();
bool otaIsInProgress();
void webServerInit();

#ifdef CONF_USE_ARDUINO_OTA
void otaInit();
void otaLoop();
#endif

#ifdef CONF_MQTT
void mqttInit();
void mqttSetPaused(bool paused);
#endif

#endif // _GAIA_NETWORK
