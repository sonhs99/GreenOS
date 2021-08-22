#include "ConsoleShell.hpp"
#include "Console.hpp"
#include "Keyboard.hpp"
#include "Utility.hpp"

static ShellCommandEntry gs_vstCommandTable[] = {
	{"help", "Show Help", kHelp},
	{"cls", "Clear Screen", kCls},
	{"totalram", "Show Total Ram Size", kShowTotalRAMSize},
	{"strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest},
	{"shutdown", "Shutdown And Reboot OS", kShutdown},
};

void kStartConsoleShell() {
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int iCommandBufferIndex = 0;
	int iCursorX, iCursorY;

	kPrintf(CONSOLESHELL_PROMPTMESSAGE);
	while(true) {
		char bKey = kGetCh();
		if(bKey == KEY_BACKSPACE) {
			if(iCommandBufferIndex > 0){
				kGetCursor(iCursorX, iCursorY);
				kPrintStringXY(iCursorX - 1, iCursorY, " ");
				kSetCursor(iCursorX - 1, iCursorY);
				iCommandBufferIndex--;
			}
		}
		else if(bKey == KEY_ENTER) {
			kPrintf("\n");
			if(iCommandBufferIndex > 0){
				vcCommandBuffer[iCommandBufferIndex] = '\0';
				kExecuteCommand(vcCommandBuffer);
			}
			kPrintf("%s", CONSOLESHELL_PROMPTMESSAGE);
			kMemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			iCommandBufferIndex = 0;
		}
		else if (bKey == KEY_LSHIFT || bKey == KEY_RSHIFT ||
				 bKey == KEY_CAPSLOCK || bKey == KEY_NUMLOCK ||
				 bKey == KEY_SCROLLLOCK);
		else {
			if(bKey == KEY_TAB) bKey = ' ';
			if(iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT){
				vcCommandBuffer[iCommandBufferIndex++] = bKey;
				kPrintf("%c", bKey);
			}
		}
	}
}

void kExecuteCommand(const char *pcCommandBuffer) {
	int iCommandBufferLength = kStrLen(pcCommandBuffer);
	int iSpaceIndex;

	for(iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
		if(pcCommandBuffer[iSpaceIndex] == ' ') break;

	for(auto &stCommand : gs_vstCommandTable) {
		int iCommandLength = kStrLen(stCommand.pcCommand);
		if(iCommandLength == iSpaceIndex &&
			kMemCmp(pcCommandBuffer, stCommand.pcCommand, iSpaceIndex) == 0){
			stCommand.pfFunction(pcCommandBuffer + iSpaceIndex + 1);
			return;
		}
	}
	kPrintf("'%s' is not found.\n", pcCommandBuffer);
}

ParameterList::ParameterList(const char *pcParameter) {
	pcBuffer = pcParameter;
	iLength = kStrLen(pcParameter);
	iCurrentPosition = 0;
}

int ParameterList::getNextParameter(char *pcParameter) {
	if(this->iLength <= iCurrentPosition) return 0;

	int i;
	for(i = iCurrentPosition; i < iLength; i++)
		if(pcBuffer[i] == ' ') break;

	kMemCpy(pcParameter, pcBuffer + iCurrentPosition, i);
	int iLength = i - iCurrentPosition;
	pcParameter[iLength] = '\0';
	iCurrentPosition = i + 1;
	return iLength;
}

void kHelp(const char *pcCommandBuffer) {
	kPrintf("GreenOS Kernel Shell Help\n");

	int iMaxCommandLength = 0;
	for(auto &stCommand: gs_vstCommandTable) {
		int iLength = kStrLen(stCommand.pcCommand);
		iMaxCommandLength = iMaxCommandLength > iLength ? iMaxCommandLength : iLength;
	}

	int iCursorX, iCursorY;
	for(auto &stCommand: gs_vstCommandTable) {
		kPrintf("%s", stCommand.pcCommand );
		kGetCursor(iCursorX, iCursorY);
		kSetCursor(iMaxCommandLength, iCursorY);
		kPrintf("  - %s\n", stCommand.pcHelp);
	}
}

void kCls(const char *pcParameterBuffer) {
	kClearScreen();
	kSetCursor(0, 1);
}

void kShowTotalRAMSize(const char *pcParameterBuffer) {
	kPrintf("Total RAM Size = %d MB\n", kGetTotalRAMSize());
}

void kStringToDecimalHexTest(const char *pcParameterBuffer) {
	char vcParameter[100];
	ParameterList stList(pcParameterBuffer);
	int iCount = 0;
	long lValue;

	while(true) {
		int iLength = stList.getNextParameter(vcParameter);
		if(iLength == 0) break;
		kPrintf("Param %d = %s, Length = %d, ", iCount + 1, vcParameter, iLength);

		if(kMemCmp(vcParameter, "0x", 2) == 0) {
			lValue = kAToI(vcParameter + 2, 16);
			kPrintf("HEX Value = %q\n", lValue);
		} else {
			lValue = kAToI(vcParameter, 10);
			kPrintf("Decimal Value = %d\n", lValue);
		}
		iCount++;
	}
}

void kShutdown(const char *pcParameterBuffer) {
	kPrintf("System Shutdown Start...\n");
	kPrintf("Press Any Key To Reboot PC...");
	kGetCh();
	kReboot();
}