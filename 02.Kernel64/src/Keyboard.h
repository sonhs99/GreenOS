#pragma once

#include "Types.h"

#define KEY_SKIPCOUNTFORPAUSE   2

#define KEY_FLAGS_UP            0x00
#define KEY_FLAGS_DOWN          0x01
#define KEY_FLAGS_EXTENDEDKEY   0x02

#define KEY_MAPPINGTABLEMAXCOUNT    89

// Pause 키를 처리하기 위해 무시해야 하는 나머지 스캔 코드의 수
#define KEY_SKIPCOUNTFORPAUSE       2

// 키 상태에 대한 플래그
#define KEY_FLAGS_UP             0x00
#define KEY_FLAGS_DOWN           0x01
#define KEY_FLAGS_EXTENDEDKEY    0x02

// 스캔 코드 매핑 테이블에 대한 매크로
#define KEY_MAPPINGTABLEMAXCOUNT    89

#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08

#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8A
#define KEY_F5          0x8B
#define KEY_F6          0x8C
#define KEY_F7          0x8D
#define KEY_F8          0x8E
#define KEY_F9          0x8F
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9A
#define KEY_PAGEDOWN    0x9B
#define KEY_INS         0x9C
#define KEY_DEL         0x9D
#define KEY_F11         0x9E
#define KEY_F12         0x9F
#define KEY_PAUSE       0xA0 

#pragma pack(push, 1)

struct KeyMappingEntry {
    u8 bNormalCode;
    u8 bCombinedCode;
};

#pragma pack(pop)

struct KeyboardManager {
    bool bShiftDown;
    bool bCapsLockDown;
    bool bNumLockDown;
    bool bScrollLockDown;

    bool bExtendedCodeIn;
    int iSkipCountForPause;
};

bool kIsOutputBufferFull();
bool kIsInputBufferFull();
bool kActivateKeyboard();
u8 kGetKeyboardScanCode();
bool kChangeKeyboardLED(bool bCapsLockDown, bool bNumLockDown, bool bScrollLockOn);
void kEnableA20Gate();
void kReboot();
bool kIsAlphabetScanCode(u8 bScanCode);
bool kIsNumberPadScanCode(u8 bScancode);
bool kIsUseCombinedCode(bool bScanCode);
void UpdateCombinedKeyStatusAndLED(u8 bScanCode);
bool kConvertScanCodeToASCIICode(u8 bScanCode, u8 pbASCIICode, bool* pbFlags);