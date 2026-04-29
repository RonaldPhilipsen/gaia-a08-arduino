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

#include "config.hpp"
#include <WiFi.h>
#include "network.hpp"
#include "sensors.hpp"
#include "logger.hpp"
#include <ESPAsyncWebServer.h>

#ifdef CONF_USE_WEB_SERVER

AsyncWebServer server(80);
AsyncEventSource serialEvents("/serial-events");

const char SERIAL_PAGE_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
    <head>
        <meta charset="utf-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <title>GAIA A08 Serial</title>
        <style>
            body { margin: 0; font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; background: #101114; color: #d8dee9; }
            .header { padding: 12px 16px; border-bottom: 1px solid #2a2e36; display: flex; justify-content: space-between; align-items: center; }
            .badge { color: #a3be8c; }
            pre { margin: 0; padding: 16px; white-space: pre-wrap; word-wrap: break-word; }
            button { background: #2e3440; color: #d8dee9; border: 1px solid #3b4252; border-radius: 6px; padding: 6px 10px; cursor: pointer; }
        </style>
    </head>
    <body>
        <div class="header">
            <div>GAIA A08 Web Serial</div>
            <div>
                <span id="state" class="badge">connecting...</span>
                <button id="clear">Clear</button>
            </div>
        </div>
        <pre id="log"></pre>
        <script>
            const logEl = document.getElementById('log');
            const stateEl = document.getElementById('state');
            const MAX_CHARS = 120000;
            function append(line) {
                logEl.textContent += line + "\n";
                if (logEl.textContent.length > MAX_CHARS) {
                    logEl.textContent = logEl.textContent.slice(logEl.textContent.length - MAX_CHARS);
                }
                window.scrollTo(0, document.body.scrollHeight);
            }

            fetch('/serial-backlog')
                .then(r => r.text())
                .then(t => { if (t) { logEl.textContent = t; window.scrollTo(0, document.body.scrollHeight); } });

            const es = new EventSource('/serial-events');
            es.addEventListener('open', () => stateEl.textContent = 'connected');
            es.addEventListener('error', () => stateEl.textContent = 'reconnecting...');
            es.addEventListener('log', (e) => append(e.data));

            document.getElementById('clear').addEventListener('click', () => {
                logEl.textContent = '';
            });
        </script>
    </body>
</html>
)rawliteral";

void webServerRealtimeHandler(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    if (!pm25.hasData())
    {
        request->send(200, "application/json", "{\"status\":\"error\",\"reason\":\"no PM2.5 data\"}");
        return;
    }

    if (!getMinimalSensorData(doc))
    {
        request->send(200, "application/json", "{\"status\":\"error\"}");
        return;
    }

    static char json_body[512];
    serializeJson(doc, json_body, sizeof(json_body));
    request->send(200, "application/json", json_body);
}

void webServerInit()
{

    server.on("/realtime", HTTP_GET, webServerRealtimeHandler);
    server.on("/serial-backlog", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", webLogBacklog());
    });

    server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", SERIAL_PAGE_HTML);
    });

    serialEvents.onConnect([](AsyncEventSourceClient *client) {
        client->send("connected", "log", millis());
    });
    webLogAttachEventSource(&serialEvents);
    server.addHandler(&serialEvents);

    server.begin();
}

#endif
