#include "ConsoleShell.hpp"
#include "Console.hpp"
#include "Keyboard.hpp"
#include "Utility.hpp"
#include "PIT.hpp"
#include "RTC.hpp"
#include "Assembly.hpp"
#include "Task.hpp"
#include "Sync.hpp"
#include "Memory.hpp"
#include "HardDisk.hpp"

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
	{"changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)", kChangeTaskPriority},
	{"tasklist", "Show Task List", kShowTaskList},
	{"killtask", "End Task, ex)killtask i(ID)", kKillTask},
	{"cpuload", "Show CPU Load", kCPULoad},
	{"testmutex", "Test Mutex Function", kTestMutex},
	{"testthread", "Test Thread And Process Function", kTestThread},
	{"showmatrix", "Show Matrix Screen", kShowMatrix},
	{"testpie", "Test PIE Calculation", kTestPIE},

	{"dynamicmeminfo", "Show Dynamic Memory Information", kShowDynamicMemoryInformation},
	{"testseqalloc", "Test Sequential Alloction & Free", kTestSequentialAllocation},
	{"testranalloc", "Test Random Allocation & Free", kTestRandomAllocation},
	{"hddinfo", "Show HDD Information", kShowHDDInformation},
	{"readsector", "Read HDD Sector, ex)readsector 0(LBA) 10(Count)", kReadSector},
	{"writesector", "Write HDD Sector, ex)writesector 0(LBA) 10(Count)", kWriteSector},
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

static void kHelp(const char *pcCommandBuffer) {
	kPrintf("GreenOS Kernel Shell Help\n");

	int iMaxCommandLength = 0;
	for(auto &stCommand: gs_vstCommandTable) {
		int iLength = kStrLen(stCommand.pcCommand);
		iMaxCommandLength = iMaxCommandLength > iLength ? iMaxCommandLength : iLength;
	}

	int iCursorX, iCursorY, i = 0;
	for(auto &stCommand: gs_vstCommandTable) {
		kPrintf("%s", stCommand.pcCommand );
		kGetCursor(iCursorX, iCursorY);
		kSetCursor(iMaxCommandLength, iCursorY);
		kPrintf("  - %s\n", stCommand.pcHelp);
		if((i != 0) && ((i % 20) == 0)) {
			kPrintf("Press any key to continue... ('q' is exit) : ");
			if(kGetCh() == 'q') {
				kPrintf("\n");
				break;
			}
			kPrintf("\n");
		}
		i++;
	}
}

static void kCls(const char *pcParameterBuffer) {
	kClearScreen();
	kSetCursor(0, 1);
}

static void kShowTotalRAMSize(const char *pcParameterBuffer) {
	kPrintf("Total RAM Size = %d MB\n", kGetTotalRAMSize());
}

static void kStringToDecimalHexTest(const char *pcParameterBuffer) {
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

static void kShutdown(const char *pcParameterBuffer) {
	kPrintf("System Shutdown Start...\n");
	kPrintf("Press Any Key To Reboot PC...");
	kGetCh();
	kReboot();
}

static void kSetTimer(const char *pcParameterBuffer) {
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

static void kWaitUsingPIT(const char *pcParameterBuffer) {
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

static void kReadTimeStampCounter(const char *pcParameterBuffer) {
	kPrintf("Time Stamp Counter = %q\n", kReadTSC());
}

static void kMeasureProcessorSpeed(const char *pcParameterBuffer) {
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

static void kShowDateAndTime(const char *pcParameterBuffer) {
	u8 bSecond, bMinute, bHour;
	u8 bDayOfWeek, bDayOfMonth, bMonth;
	u16 wYear;

	kReadRTCTime(bHour, bMinute, bSecond);
	kReadRTCDate(wYear, bMonth, bDayOfMonth, bDayOfWeek);
	
	kPrintf("Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, kConvertDayOfWeekToString(bDayOfWeek));
	kPrintf("Time: %d:%d:%d\n", bHour, bMinute, bSecond);
}

static void kTestTask1() {
	u8 bData;
	int i = 0, iX = 0, iY = 0, iMargin;
	Charactor *pstScreen = (Charactor*)CONSOLE_VIDEOMEMORYADDRESS;
	Task *pstRunningTask = kGetRunningTask();
	iMargin = (pstRunningTask->qwID & 0xFFFFFFFF) % 10 + 1;

	for(int j = 0; j < 20000; j++) {
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
	// kExitTask();
}

static void kTestTask2() {
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

static void kCreateTestTask(const char *pcParameterBuffer) {
	ParameterList stList(pcParameterBuffer);
	char vcType[30];
	char vcCount[30];
	int i;

	stList.getNextParameter(vcType);
	stList.getNextParameter(vcCount);

	switch(kAToI(vcType, 10)) {
	case 1:
		for(i = 0; i < kAToI(vcCount, 10); i++)
			if(kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, u64(kTestTask1)) == nullptr) break;
		kPrintf("Task1 %d Created\n", i);
		break;
	case 2:
	default:
		for(i = 0; i < kAToI(vcCount, 10); i++)
			if(kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, u64(kTestTask2)) == nullptr) break;
		kPrintf("Task2 %d Created\n", i);
	}
}

static void kChangeTaskPriority(const char *pcParameterBuffer) {
	ParameterList stList(pcParameterBuffer);
	char vcID[30];
	char vcPriority[30];

	stList.getNextParameter(vcID);
	stList.getNextParameter(vcPriority);

	u64 qwID = (kMemCmp(vcID, "0x", 2) == 0) ? kAToI(vcID + 2, 16) : kAToI(vcID, 10);
	u8 bPriority = kAToI(vcPriority, 10);

	kPrintf("Change Task Priority ID [0x%q] Priority[%d]", qwID, bPriority);
	if(kChangePriority(qwID, bPriority)) kPrintf("Success\n");
	else kPrintf("Fail\n");
}

static void kShowTaskList(const char *pcParameterBuffer) {
	Task* pstTask;
	int iCount = 0;
	for(int i = 0; i < TASK_MAXCOUNT; i++) {
		pstTask = kGetTaskInTCBPool(i);
		if(pstTask->qwID >> 32 != 0){
			if((iCount != 0) && ((iCount % 10) == 0)) {
				kPrintf("Press any key to continue... ('q' is exit) : ");
				if(kGetCh() == 'q') {
					kPrintf("\n");
					break;
				}
				kPrintf("\n");
			}
			kPrintf("[%d] Task ID[0x%q], Priority[%d], Flags[0x%q], Thread[%d]\n",
					1 + iCount++, pstTask->qwID, GETPRIORITY(pstTask->qwFlags), pstTask->qwFlags, pstTask->stChildThreadList.ItemCount());
			kPrintf("     Parent PID[0x%q], Memory Address[0x%q], Size[0x%Q]\n",
					pstTask->qwParentProcessID, pstTask->pvMemoryAddress, pstTask->qwMemorySize);
		}
	}
}

static void kKillTask(const char *pcParameterBuffer) {
	ParameterList stList(pcParameterBuffer);
	char vcID[30];
	stList.getNextParameter(vcID);
	
	u64 qwID = (kMemCmp(vcID, "0x", 2) == 0) ? kAToI(vcID + 2, 16) : kAToI(vcID, 10);
	if(qwID != 0xFFFFFFFF) {
		Task *pstTask = kGetTaskInTCBPool(GETTCBOFFSET(qwID));
		qwID = pstTask->qwID;
		if((qwID >> 32) != 0 && ((pstTask->qwFlags & TASK_FLAGS_SYSTEM) == 0x00)) {
			kPrintf("Kill Task ID [0x%q] ", qwID);
			if(kEndTask(qwID)) kPrintf("Success\n");
			else kPrintf("Fail\n");
		} else kPrintf("Task does not exist or task is system task\n");
	} else {
		for(int i = 2; i < TASK_MAXCOUNT; i++) {
			Task* pstTask = kGetTaskInTCBPool(i);
			qwID = pstTask->qwID;
			if(((qwID >> 32) != 0) && ((pstTask->qwFlags & TASK_FLAGS_SYSTEM) == 0x00)) {
				kPrintf("Kill Task ID [0x%q] ", qwID);
				if(kEndTask(qwID)) kPrintf("Success\n");
				else kPrintf("Fail\n");
			}
		}
	}
}

static void kCPULoad(const char *pcParameterBuffer) {
	kPrintf("Processor Load : %d%%\n", kGetProcessorLoad());
}

static Mutex gs_stMutex;
static volatile u64 gs_qwAdder;

static void kPrintNumberTask() {
	u64 qwTickCount = kGetTickCount();
	while((kGetTickCount() - qwTickCount) < 50) kSchedule();
	for(int i = 0; i < 5; i++){
		gs_stMutex.lock();
		kPrintf("Task ID [0x%q] Value[%d]\n", kGetRunningTask()->qwID, gs_qwAdder);
		gs_qwAdder += 1;
		gs_stMutex.unlock();
		for(int j = 0; j < 30000; j++);
	}
	qwTickCount = kGetTickCount();
	while((kGetTickCount() - qwTickCount) < 1000) kSchedule();
	kExitTask();
}

static void kTestMutex(const char *pcParameterBuffer) {
	gs_qwAdder = 1;
	gs_stMutex = Mutex();
	int i;
	for(i = 0; i < 3; i++) 
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, u64(kPrintNumberTask));
	kPrintf("Wait Until %d Task End\n", i);
	kGetCh();
}

static void kCreateThreadTask() {
	for(int i = 0; i < 3; i++)
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, u64(kTestTask2));
	while(true) kSleep(1);
}

static void kTestThread(const char *pcParameterBuffer) {
	Task* pstProcess = kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, (void *)0xEEEEEEEE, 0x1000, u64(kCreateThreadTask));
	if(pstProcess != nullptr)
		kPrintf("Process [0x%q] Create Success\n", pstProcess->qwID);
	else kPrintf("Process Create Fail\n");
}

u64 kRandom() {
	static volatile u64 s_qwRandomValue = 0;
	s_qwRandomValue = (s_qwRandomValue * 412153 + 5571031) >> 16;
	return s_qwRandomValue;
}

static void kDropCharactorThread() {
	char vcText[2] = {0,};
	int iX = kRandom() % CONSOLE_WIDTH;

	while(true) {
		kSleep(kRandom() % 20);
		if((kRandom() % 20) < 15) {
			vcText[0] = ' ';
			for(int i = 0; i < CONSOLE_HEIGHT - 1; i++) {
				kPrintStringXY(iX, i, vcText);
				kSleep(50);
			}
		} else
			for(int i = 0; i < CONSOLE_HEIGHT - 1; i++) {
				vcText[0] = i + kRandom();
				kPrintStringXY(iX, i, vcText);
				kSleep(50);
			}
	}
}

static void kMatrixProcess() {
	int i;
	for(i = 0; i < 300; i++) {
		if(kCreateTask(TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0, u64(kDropCharactorThread)) == nullptr)
			break;
		kSleep(kRandom() % 5 + 5);
	}
	kPrintf("%d Thread is created\n", i);
	kGetCh();
}

static void kShowMatrix(const char *pcParameterBuffer) {
	Task* pstProcess = kCreateTask(TASK_FLAGS_PROCESS | TASK_FLAGS_LOW,
									(void*) 0xE00000, 0xE00000, u64(kMatrixProcess));
	if(pstProcess != nullptr) {
		kPrintf("Matrix Process [0x%q] Create Success\n", pstProcess->qwID);
		while((pstProcess->qwID >> 32) != 0) kSleep(100);
	} else kPrintf("Matrix Process Create Fail\n");
}

static void kFPUTestTask() {
	u64 qwCount = 0;
	Charactor *pstScreen = (Charactor*) CONSOLE_VIDEOMEMORYADDRESS;
	Task *pstRunningTask = kGetRunningTask();
	char vcData[4] = {'-', '\\', '|', '/'};
	int iOffset = (pstRunningTask->qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));
	while(true) {
		double dValue1 = 1, dValue2 = 1;
		for(int i = 0; i < 10; i++) {
			u64 qwRandomValue = kRandom();
			dValue1 *= (double) qwRandomValue;
			dValue2 *= (double) qwRandomValue;
			kSleep(1);
			qwRandomValue = kRandom();
			dValue1 /= (double) qwRandomValue;
			dValue2 /= (double) qwRandomValue;
		}
		if(dValue1 != dValue2) {
			kPrintf("Value Is Not Same [%f] != [%f]", dValue1, dValue2);
			break;
		}
		qwCount++;
		pstScreen[iOffset] = {
			.bCharactor = u8(vcData[qwCount % 4]),
			.bAttribute = u8((iOffset % 15) + 1)
		};
	}
}

static void kTestPIE(const char *pcParameterBuffer) {
	kPrintf("PIE Calculation Test\n");
	kPrintf("Result: 355 / 133 = ");
	double dResult = double(355.0/113.0);
	kPrintf("%d.%d%d\n", u64(dResult), u64(dResult * 10) % 10, u64(dResult * 100) % 10);
	for(int i = 0; i < 100; i++)
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, u64(kFPUTestTask));
}

static void kShowDynamicMemoryInformation(const char *pcParameterBuffer) {
	u64 qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize;
	kGetDynamicMemoryInformation(qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize);
	kPrintf("================ Dynamic Memory Information ==================\n");
	kPrintf("Start Address: [0x%q]\n", qwStartAddress);
	kPrintf("Total Size:    [0x%q]byte, [%d]MB\n", qwTotalSize, qwTotalSize / 1024 / 1024);
	kPrintf("Meta Size:     [0x%q]byte, [%d]KB\n", qwMetaSize, qwMetaSize / 1024);
	kPrintf("Used Size:     [0x%q]byte, [%d]KB\n", qwUsedSize, qwUsedSize / 1024);
}

static void kTestSequentialAllocation(const char *pcParameterBuffer) {
	DynamicMemory& pstMemory = kGetDynamicMemoryManager();
	kPrintf("============ Dynamic Memory Test ============\n");
	for(int i = 0; i < pstMemory.iMaxLevelCount; i++) {
		kPrintf("Block List [%d] Test Start\n", i);
		kPrintf("Allocation And Compare: ");
		for(int j = 0; j < (pstMemory.iBlockCountOfSmallestBlock >> i); j++){
			u64 *pqwBuffer = (u64*)kAllocateMemory(DYNAMICMEMORY_MIN_SIZE << i);
			if(pqwBuffer == nullptr) {
				kPrintf("\nAllocation Fail\n");
				return;
			}
			for(int k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; k++)
				pqwBuffer[k] = k;
			for(int k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; k++)
				if(pqwBuffer[k] != k) {
					kPrintf("Compare Fail\n");
					return;
				}
			kPrintf(".");
		}
		kPrintf("\nFree: ");
		for(int j = 0; j < (pstMemory.iBlockCountOfSmallestBlock >> i); j++) {
			if(!kFreeMemory((void*)(pstMemory.qwStartAddress + (DYNAMICMEMORY_MIN_SIZE << i) * j))) {
				kPrintf("Free Fail\n");
				return;
			}
			kPrintf(".");
		}
		kPrintf("\n");
	}
	kPrintf("Test Complete.\n");
}

static void kRandomAllocationTask() {
	Task *pstTask = kGetRunningTask();
	char vcBuffer[200];
	int iY = pstTask->qwID % 15 + 9;
	for(int j = 0; j < 10; j++) {
		u8 *pbAllocationBuffer;
		u64 qwMemorySize;
		do {
			qwMemorySize = ((kRandom() % (32 * 1024)) + 1) * 1024;
			pbAllocationBuffer = (u8 *)kAllocateMemory(qwMemorySize);
			if(pbAllocationBuffer == nullptr) kSleep(1);
		} while(pbAllocationBuffer == nullptr);
		kSPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Allocation Success", pbAllocationBuffer, qwMemorySize);
		kPrintStringXY(20, iY, vcBuffer);
		for(int i = 0; i < qwMemorySize / 2; i++) {
			pbAllocationBuffer[i] = kRandom() % 0xFF;
			pbAllocationBuffer[i + (qwMemorySize / 2)] = pbAllocationBuffer[i];
		}
		kSleep(200);
		kSPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Verify...   ", pbAllocationBuffer, qwMemorySize);
		kPrintStringXY(20, iY, vcBuffer);
		for(int i = 0; i < qwMemorySize / 2; i++)
			if(pbAllocationBuffer[i] != pbAllocationBuffer[i + qwMemorySize / 2]) {
				kPrintf("Task ID[0x%Q] Verify Fail\n", pstTask->qwID);
				kExitTask();
			}
		kFreeMemory(pbAllocationBuffer);
		kSleep(100);
	}
	kExitTask();
}

static void kTestRandomAllocation(const char *pcParameterBuffer) {
	for(int i = 0; i < 100; i++)
		kCreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, u64(kRandomAllocationTask));
}

static void kShowHDDInformation(const char *pcParameterBuffer) {
	HDDInformation stHDD;
	char vcBuffer[100];
	if(!kReadHDDInformation(true, true, stHDD))	{
		kPrintf("HDD Information Read Fail\n");
		return;
	}

	kPrintf("============= Primary Master HDD Information =============\n");

	kMemCpy(vcBuffer, stHDD.vwModelNumber, sizeof(stHDD.vwModelNumber));
	vcBuffer[sizeof(stHDD.vwModelNumber) - 1] = '\0';
	kPrintf("Model Number:\t %s\n", vcBuffer);

	kPrintf("Head Count:\t %d\n", stHDD.wNumberOfHead);
	kPrintf("Cylinder Count:\t %d\n", stHDD.wNumberOfCylinder);
	kPrintf("Sector Count:\t %d\n", stHDD.wNumberOfSectorPerCylinder);

	kPrintf("Total Sector:\t %d Sector, %dMB\n", stHDD.dwTotalSector, stHDD.dwTotalSector / 2 / 1024);
}

static void kReadSector(const char *pcParameterBuffer) {
	ParameterList stList(pcParameterBuffer);
	char vcLBA[50], vcSectorCount[50];
	if(stList.getNextParameter(vcLBA) == 0 || stList.getNextParameter(vcSectorCount) == 0) {
		kPrintf("ex) readsector 0(LBA) 10(Count)\n");
		return;
	}

	u32 dwLBA = kAToI(vcLBA, 10);
	int iSectorCount = kAToI(vcSectorCount, 10);

	char *pcBuffer = (char*)kAllocateMemory(iSectorCount * 512);
	bool bExit = false;
	if(kReadHDDSector(true, true, dwLBA, iSectorCount, pcBuffer) == iSectorCount) {
		kPrintf("LBA[%d], [%d] Sector Read Success", dwLBA, iSectorCount);
		for(int j = 0; j < iSectorCount; j++) {
			for(int i = 0; i < 512; i++) {
				if(!((j == 0) && (i == 0)) && ((i % 256) == 0)) {
					kPrintf("\nPress any key to continue... ('q' is exit) : ");
					if(kGetCh() == 'q') {
						bExit = true;
						break;
					}
				}
				if((i%16 == 0))
					kPrintf("\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i);
				u8 bData = pcBuffer[j * 512 + i] & 0xFF;
				if(bData < 16) kPrintf("0");
				kPrintf("%x ", bData);
			}
			if(bExit) break;
		}
		kPrintf("\n");
	} else kPrintf("Read Fail\n");
	kFreeMemory(pcBuffer);
}

static void kWriteSector(const char *pcParameterBuffer) {
	ParameterList stList(pcParameterBuffer);
	char vcLBA[50], vcSectorCount[50];
	if(stList.getNextParameter(vcLBA) == 0 || stList.getNextParameter(vcSectorCount) == 0) {
		kPrintf("ex) writesector 0(LBA) 10(Count)\n");
		return;
	}

	u32 dwLBA = kAToI(vcLBA, 10);
	int iSectorCount = kAToI(vcSectorCount, 10);

	static u32 s_dwWriteCount = 0;
	s_dwWriteCount++;
	char *pcBuffer = (char*)kAllocateMemory(iSectorCount * 512);
	for(int j = 0; j < iSectorCount; j++) {
		for(int i = 0; i < 512; i += 8) {
			*(u32 *) &(pcBuffer[j * 512 + i]) = dwLBA + j;
			*(u32 *) &(pcBuffer[j * 512 + i + 4]) = s_dwWriteCount;
		}
	}

	if(kWriteHDDSector(true, true, dwLBA, iSectorCount, pcBuffer) != iSectorCount) {
		kPrintf("Write Fail\n");
		return;
	}
	kPrintf("LBA [%d], [%d] Sector Read Success", dwLBA, iSectorCount);

	bool bExit = false;
	for(int j = 0; j < iSectorCount; j++) {
		for(int i = 0; i < 512; i++) {
			if(!((j == 0) && (i == 0)) && ((i % 256) == 0)) {
				kPrintf("\nPress any key to continue... ('q' is exit) : ");
				if(kGetCh() == 'q') {
					bExit = true;
					break;
				}
			}
			if((i%16 == 0))
				kPrintf("\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i);
			u8 bData = pcBuffer[j * 512 + i] & 0xFF;
			if(bData < 16) kPrintf("0");
			kPrintf("%x ", bData);
		}
		if(bExit) break;
	}
	kPrintf("\n");
	kFreeMemory(pcBuffer);
}