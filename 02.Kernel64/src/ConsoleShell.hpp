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

static void kHelp(const char *pcParameterBuffer);
static void kCls(const char *pcParameterBufer);
static void kShowTotalRAMSize(const char *pcParameterBuffer);
static void kStringToDecimalHexTest(const char *pcParameterBuffer);
static void kShutdown(const char *pcParameterBuffer);

static void kSetTimer(const char *pcParameterBuffer);
static void kWaitUsingPIT(const char *pcParameterBuffer);
static void kReadTimeStampCounter(const char *pcParameterBuffer);
static void kMeasureProcessorSpeed(const char *pcParameterBuffer);
static void kShowDateAndTime(const char *pcParameterBuffer);
static void kCreateTestTask(const char *pcParameterBuffer);

static void kChangeTaskPriority(const char *pcParameterBuffer);
static void kShowTaskList(const char *pcParameterBuffer);
static void kKillTask(const char *pcParameterBuffer);
static void kCPULoad(const char *pcParameterBuffer);

static void kTestMutex(const char *pcParameterBuffer);
static void kTestThread(const char *pcParameterBuffer);
static void kShowMatrix(const char *pcParameterBuffer);

static void kTestPIE(const char *pcParameterBuffer);