#pragma once

#include "Types.hpp"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          ">"

using CommandFunction = void(const char*);

#pragma pack(push, 1)

struct ShellCommandEntry {
    char* pcCommand;
    char* pcHelp;
    CommandFunction pfFunction;
};

struct ParameterList {
    const char* pcBuffer;
    int iLength;
    int iCurrentPosition;
};

#pragma pack(pop)