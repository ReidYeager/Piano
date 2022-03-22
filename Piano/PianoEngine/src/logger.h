
#ifndef PIANO_LOGGER_H_
#define PIANO_LOGGER_H_

#include "defines.h"
#include "platform/platform.h"

#include <stdarg.h>
#include <stdio.h>

namespace Piano {
  enum LogTypes
  {
    Log_Info,
    Log_Debug,
    Log_Warning,
    Log_Error,
    Log_Fatal
  };

  inline void ConsoleLogMessage(Piano::LogTypes _type, const char* _message, ...)
  {
    // Limit 65,535 characters per message
    const u16 length = 0xFFFF;
    char* outMessage = new char[length];
    Piano::MemoryZero(outMessage, length);

    va_list args;
    va_start(args, _message);
    vsnprintf(outMessage, length, _message, args);
    va_end(args);

    Piano::Platform::PrintToConsole(outMessage, _type);

    delete[](outMessage);
  }
}

#ifdef PIANO_DEBUG
#define PianoLogInfo(message, ...)                                 \
{                                                                  \
  Piano::ConsoleLogMessage(Piano::Log_Info, message, __VA_ARGS__); \
  Piano::ConsoleLogMessage(Piano::Log_Info, "\n"); \
}

#define PianoLogDebug(message, ...)                                 \
{                                                                   \
  Piano::ConsoleLogMessage(Piano::Log_Debug, message, __VA_ARGS__); \
  Piano::ConsoleLogMessage(Piano::Log_Debug, "\n");                 \
}

#define PianoLogWarning(message, ...)                                 \
{                                                                     \
  Piano::ConsoleLogMessage(Piano::Log_Warning, message, __VA_ARGS__); \
  Piano::ConsoleLogMessage(Piano::Log_Warning, "\n");                 \
}
#else
#define PianoLogInfo(message, ...)
#define PianoLogDebug(message, ...)
#define PianoLogWarning(message, ...)
#endif // PIANO_DEBUG

#define PianoLogError(message, ...)                                 \
{                                                                   \
  Piano::ConsoleLogMessage(Piano::Log_Error, message, __VA_ARGS__); \
  Piano::ConsoleLogMessage(Piano::Log_Error, "\n");                 \
}

#define PianoLogFatal(message, ...)                                 \
{                                                                   \
  Piano::ConsoleLogMessage(Piano::Log_Fatal, message, __VA_ARGS__); \
  Piano::ConsoleLogMessage(Piano::Log_Fatal, "\n");                 \
}

#endif // !PIANO_LOGGER_H_
