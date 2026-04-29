#include "logger.hpp"

#include <stdarg.h>
#include <ESPAsyncWebServer.h>

namespace
{
constexpr size_t LOG_BUFFER_LINES = 120;
String logLines[LOG_BUFFER_LINES];
size_t logHead = 0;
size_t logCount = 0;
String partialLine;
portMUX_TYPE logMux = portMUX_INITIALIZER_UNLOCKED;
AsyncEventSource *logEvents = nullptr;

void pushLogLine(const String &line)
{
    if (line.length() == 0)
    {
        return;
    }

    portENTER_CRITICAL(&logMux);
    logLines[logHead] = line;
    logHead = (logHead + 1) % LOG_BUFFER_LINES;
    if (logCount < LOG_BUFFER_LINES)
    {
        logCount++;
    }
    portEXIT_CRITICAL(&logMux);

    if (logEvents != nullptr)
    {
        logEvents->send(line.c_str(), "log", millis());
    }
}

void consumeForBuffer(const String &text)
{
    int start = 0;
    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            String chunk = text.substring(start, i);
            String completed = partialLine + chunk;
            partialLine = "";
            if (completed.length() > 0)
            {
                pushLogLine(completed);
            }
            start = i + 1;
        }
    }

    if (start < text.length())
    {
        partialLine += text.substring(start);
    }
}
} // namespace

void webLogAttachEventSource(AsyncEventSource *events)
{
    logEvents = events;
}

String webLogBacklog()
{
    String output;
    portENTER_CRITICAL(&logMux);
    size_t start = (logHead + LOG_BUFFER_LINES - logCount) % LOG_BUFFER_LINES;
    for (size_t i = 0; i < logCount; i++)
    {
        const String &line = logLines[(start + i) % LOG_BUFFER_LINES];
        output += line;
        output += "\n";
    }
    if (partialLine.length() > 0)
    {
        output += partialLine;
        output += "\n";
    }
    portEXIT_CRITICAL(&logMux);
    return output;
}

void webLogPrint(const String &message)
{
    Serial.print(message);
    consumeForBuffer(message);
}

void webLogPrintln(const String &message)
{
    Serial.println(message);
    consumeForBuffer(message + "\n");
}

void webLogPrintf(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (written <= 0)
    {
        return;
    }

    if (written >= static_cast<int>(sizeof(buffer)))
    {
        buffer[sizeof(buffer) - 1] = '\0';
    }

    Serial.print(buffer);
    consumeForBuffer(String(buffer));
}
