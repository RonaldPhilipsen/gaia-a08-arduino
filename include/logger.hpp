#ifndef _GAIA_LOGGER
#define _GAIA_LOGGER

#include <Arduino.h>

class AsyncEventSource;

void webLogAttachEventSource(AsyncEventSource *events);
String webLogBacklog();

void webLogPrint(const String &message);
void webLogPrintln(const String &message);
void webLogPrintf(const char *fmt, ...);

#endif // _GAIA_LOGGER
