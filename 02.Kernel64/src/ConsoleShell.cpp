#include "ConsoleShell.hpp"
#include "Console.hpp"
#include "Keyboard.hpp"
#include "Utility.hpp"
#include "PIT.hpp"
#include "RTC.hpp"
#include "Assembly.hpp"

static ShellCommandEntry gs_vstCommandTable[] = {
	{"help", "Show Help", kHelp},
	{"cls", "Clear Screen", kCls},
	{"totalram", "Show Total Ram Size", kShowTotalRAMSize},
	{"strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest},
	{"shutdown", "Shutdown And Reboot OS", kShutdown},

	{"settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
	{"wait", "Wait ms Using PIT, ex)wait 100ms", kWaitUsingPIT},
	{"rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter },
	{"cpuspeed", "Measure Proccessor Speed", kMeasureProcessorSpeed},
	{"date", "Show Date And Time", kShowDateAndTime},
	{"createtask", "Create Task, ex)createtask 1(type) 10(count)", kCreateTestTask},
	{"test", "Test features", kTest},
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

void kSetTimer(const char *pcParameterBuffer) {
	char vcParameter[100];
	ParameterList stList(pcParameterBuffer);

	if(stList.getNextParameter(vcParameter) == 0){
		kPrintf("ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	long lValue = kAToI(vcParameter, 10);

	if(stList.getNextParameter(vcParameter) == 0){
		kPrintf("ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	bool bPeriodic = kAToI(vcParameter, 10);

	kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
	kPrintf("Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic);
}

void kWaitUsingPIT(const char *pcParameterBuffer) {
	char vcParameter[100];
	ParameterList stList(pcParameterBuffer);

	if(stList.getNextParameter(vcParameter) == 0) {
		kPrintf("ex)wait 100(ms)\n");
		return;
	}

	long lMillisecond = kAToI(vcParameter, 10);
	kPrintf("%d ms Sleep Start...\n");

	kDisableInterrupt();
	for(int i = 0; i < lMillisecond / 30; i++) 
		kWaitUsingDirectPIT(MSTOCOUNT(30));
	kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
	kEnableInterrupt();
	kPrintf("%d ms Sleep Complete\n", lMillisecond);

	kInitializePIT(MSTOCOUNT(1), true);
}

void kReadTimeStampCounter(const char *pcParameterBuffer) {
	kPrintf("Time Stamp Counter = %q\n", kReadTSC());
}

void kMeasureProcessorSpeed(const char *pcParameterBuffer) {
	kPrintf("Now Measuring.");
	
	kDisableInterrupt();
	u64 qwTotalTSC = 0;
	for(int i = 0; i < 200; i++) {
		u64 qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT(MSTOCOUNT(50));
		qwTotalTSC += kReadTSC() - qwLastTSC;
		kPrintf(".");
	}
	kInitializePIT(MSTOCOUNT(1), true);
	kEnableInterrupt();
	kPrintf("\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}

void kShowDateAndTime(const char *pcParameterBuffer) {
	u8 bSecond, bMinute, bHour;
	u8 bDayOfWeek, bDayOfMonth, bMonth;
	u16 wYear;

	kReadRTCTime(bHour, bMinute, bSecond);
	kReadRTCDate(wYear, bMonth, bDayOfMonth, bDayOfWeek);
	
	kPrintf("Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, kConvertDayOfWeekToString(bDayOfWeek));
	kPrintf("Time: %d:%d:%d\n", bHour, bMinute, bSecond);
}

void kTestTask1() {
	u8 bData;
	int i = 0, iX = 0, iY = 0, iMargin;
	Charactor *pstScreen = (Charactor*)CONSOLE_VIDEOMEMORYADDRESS;
	Task *pstRunningTask = kGetRunningTask();
	iMargin = (pstRunningTask->qwID & 0xFFFFFFFF) % 10 + 1;

	while(true) {
		switch(i) {
		case 0:
			iX++;
			if(iX >= (CONSOLE_WIDTH - iMargin)) i = 1;
			break;
		case 1:
			iY++;
			if(iY >= (CONSOLE_HEIGHT - iMargin)) i = 2;
			break;
		case 2:
			iX--;
			if(iX < iMargin) i = 3;
			break;
		case 3:
			iY--;
			if(iY < iMargin) i = 0;
			break;
		}

		pstScreen[iY * CONSOLE_WIDTH + iX] = {
			.bCharactor = bData,
			.bAttribute = u8(bData & 0x0F)
		};
		bData++;

		// kSchedule();
	}
}

void kTestTask2() {
	int i = 0, iOffset;
	Charactor *pstScreen = (Charactor*) CONSOLE_VIDEOMEMORYADDRESS;
	Task *pstRunningTask = kGetRunningTask();
	char vcData[4] = {'-', '\\', '|', '/'};
	iOffset = (pstRunningTask->qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));
	while(true) {
		pstScreen[iOffset] = {
			.bCharactor = u8(vcData[i % 4]),
			.bAttribute = u8((iOffset % 15) + 1)
		};
		i++;
		// kSchedule();
	}
}

void kCreateTestTask(const char *pcParameterBuffer) {
	ParameterList stList(pcParameterBuffer);
	char vcType[30];
	char vcCount[30];
	int i;

	stList.getNextParameter(vcType);
	stList.getNextParameter(vcCount);

	switch(kAToI(vcType, 10)) {
	case 1:
		for(i = 0; i < kAToI(vcCount, 10); i++)
			if(kCreateTask(0, u64(kTestTask1)) == nullptr) break;
		kPrintf("Task1 %d Created\n", i);
		break;
	case 2:
	default:
		for(i = 0; i < kAToI(vcCount, 10); i++)
			if(kCreateTask(0, u64(kTestTask2)) == nullptr) break;
		kPrintf("Task2 %d Created\n", i);
	}
}

void kTest(const char *pcParameterBuffer) {
	Task *pstStartAddress = (Task*) TASK_TCBPOOLADDRESS;
	for (int i = 0; i < 10; i++)
		kPrintf("%p\n", pstStartAddress[i].qwID);
	kPrintf("Test Completed.\n");
}