#pragma once

#include "Types.hpp"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          ">"

using CommandFunction = void(*)(const char*);

#pragma pack(push, 1)

struct ShellCommandEntry {
    const char* pcCommand;
    const char* pcHelp;
    CommandFunction pfFunction;
};

struct ParameterList {
    const char* pcBuffer;
    int iLength;
    int iCurrentPosition;
	ParameterList(const char *pcParameter);
	int getNextParameter(char* pcParameter);
};

#pragma pack(pop)

void kStartConsoleShell();
void kExecuteCommand(const char *pcCommandBuffer);

void kHelp(const char *pcParameterBuffer);
void kCls(const char *pcParameterBufer);
void kShowTotalRAMSize(const char *pcParameterBuffer);
void kStringToDecimalHexTest(const char *pcParameterBuffer);
void kShutdown(const char *pcParameterBuffer);

void kSetTimer(const char *pcParameterBuffer);
void kWaitUsingPIT(const char *pcParameterBuffer);
void kReadTimeStampCounter(const char *pcParameterBuffer);
void kMeasureProcessorSpeed(const char *pcParameterBuffer);
void kShowDateAndTime(const char *pcParameterBuffer);