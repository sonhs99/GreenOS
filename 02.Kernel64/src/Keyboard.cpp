#include "Types.hpp"
#include "Keyboard.hpp"
#include "Assembly.hpp"

void kPrintString(int, int, const char *);

bool kIsOutputBufferFull() {
    if(kInPortByte(0x64) & 0x01) return true;
    return false;
}

bool kIsInputBufferFull() {
    if(kInPortByte(0x64) & 0x02) return true;
    return false;
}

bool kActivateKeyboard() {
    kOutPortByte(0x64, 0xAE);

    for(int i = 0; i < 0xFFFF; i++) 
        if(!kIsInputBufferFull()) break;
    kOutPortByte(0x60, 0xF4);

    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 0xFFFF; j++)
            if(kIsOutputBufferFull()) break;
        if(kInPortByte(0x60) == 0xFA) return true;
    }
    return false;
}

u8 kGetKeyboardScanCode() {
    while(!kIsOutputBufferFull());
    return kInPortByte(0x60);
}

bool kChangeKeyboardLED(bool bCapsLockOn, bool bNumLockOn, bool bScrollLockOn) {
    for(int i = 0; i < 0xFFFF; i++)
        if(!kIsInputBufferFull()) break;

    kOutPortByte(0x60, 0xED);
    for(int i = 0; i < 0xFFFF; i++)
        if(!kIsInputBufferFull()) break;
    
    int i;
    for(i = 0; i < 100; i++) {
        for(int j = 0; j < 0xFFFF; j++)
            if(kIsOutputBufferFull()) break;
        if(kInPortByte(0x60) == 0xFA) break;
    }
    if(i >= 100) return false;
    kOutPortByte(0x60, (bCapsLockOn << 2) | (bNumLockOn << 1 | bScrollLockOn));

    for(int i = 0; i < 0xFFFF; i++)
        if(!kIsInputBufferFull()) break;

    for(i = 0; i < 100; i++) {
        for(int j = 0; j < 0xFFFF; j++)
            if(kIsOutputBufferFull()) break;
        if(kInPortByte(0x60) == 0xFA) break;
    }
    if(i >= 100) return false;

    return true;
}

void kEnableA20Gate() {
    kOutPortByte(0x64, 0xD0);
    for(int i = 0; i < 0xFFFF; i++)
        if(kIsOutputBufferFull()) break;
    
    u8 bOutputPortData = kInPortByte(0x60);
    bOutputPortData |= 0x01;
    
    for(int i = 0; i < 0xFFFF; i++)
        if(!kIsInputBufferFull()) break;
    kOutPortByte(0x64, 0xD1);
    kOutPortByte(0x60, bOutputPortData);
}

void kReboot() {
    for(int i = 0; i < 0xFFFF; i++)
        if(!kIsInputBufferFull()) break;
    kOutPortByte(0x64, 0xD1);
    kOutPortByte(0x60, 0x00);
    while(true);
}

static KeyboardManager gs_stKeyboardManager = {0, };

static KeyMappingEntry gs_vstKeyMappingTable[ KEY_MAPPINGTABLEMAXCOUNT ] =
{
    /*  0   */  {   KEY_NONE        ,   KEY_NONE        },
    /*  1   */  {   KEY_ESC         ,   KEY_ESC         },
    /*  2   */  {   '1'             ,   '!'             },
    /*  3   */  {   '2'             ,   '@'             },
    /*  4   */  {   '3'             ,   '#'             },
    /*  5   */  {   '4'             ,   '$'             },
    /*  6   */  {   '5'             ,   '%'             },
    /*  7   */  {   '6'             ,   '^'             },
    /*  8   */  {   '7'             ,   '&'             },
    /*  9   */  {   '8'             ,   '*'             },
    /*  10  */  {   '9'             ,   '('             },
    /*  11  */  {   '0'             ,   ')'             },
    /*  12  */  {   '-'             ,   '_'             },
    /*  13  */  {   '='             ,   '+'             },
    /*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
    /*  15  */  {   KEY_TAB         ,   KEY_TAB         },
    /*  16  */  {   'q'             ,   'Q'             },
    /*  17  */  {   'w'             ,   'W'             },
    /*  18  */  {   'e'             ,   'E'             },
    /*  19  */  {   'r'             ,   'R'             },
    /*  20  */  {   't'             ,   'T'             },
    /*  21  */  {   'y'             ,   'Y'             },
    /*  22  */  {   'u'             ,   'U'             },
    /*  23  */  {   'i'             ,   'I'             },
    /*  24  */  {   'o'             ,   'O'             },
    /*  25  */  {   'p'             ,   'P'             },
    /*  26  */  {   '['             ,   '{'             },
    /*  27  */  {   ']'             ,   '}'             },
    /*  28  */  {   '\n'            ,   '\n'            },
    /*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
    /*  30  */  {   'a'             ,   'A'             },
    /*  31  */  {   's'             ,   'S'             },
    /*  32  */  {   'd'             ,   'D'             },
    /*  33  */  {   'f'             ,   'F'             },
    /*  34  */  {   'g'             ,   'G'             },
    /*  35  */  {   'h'             ,   'H'             },
    /*  36  */  {   'j'             ,   'J'             },
    /*  37  */  {   'k'             ,   'K'             },
    /*  38  */  {   'l'             ,   'L'             },
    /*  39  */  {   ';'             ,   ':'             },
    /*  40  */  {   '\''            ,   '\"'            },
    /*  41  */  {   '`'             ,   '~'             },
    /*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
    /*  43  */  {   '\\'            ,   '|'             },
    /*  44  */  {   'z'             ,   'Z'             },
    /*  45  */  {   'x'             ,   'X'             },
    /*  46  */  {   'c'             ,   'C'             },
    /*  47  */  {   'v'             ,   'V'             },
    /*  48  */  {   'b'             ,   'B'             },
    /*  49  */  {   'n'             ,   'N'             },
    /*  50  */  {   'm'             ,   'M'             },
    /*  51  */  {   ','             ,   '<'             },
    /*  52  */  {   '.'             ,   '>'             },
    /*  53  */  {   '/'             ,   '?'             },
    /*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
    /*  55  */  {   '*'             ,   '*'             },
    /*  56  */  {   KEY_LALT        ,   KEY_LALT        },
    /*  57  */  {   ' '             ,   ' '             },
    /*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
    /*  59  */  {   KEY_F1          ,   KEY_F1          },
    /*  60  */  {   KEY_F2          ,   KEY_F2          },
    /*  61  */  {   KEY_F3          ,   KEY_F3          },
    /*  62  */  {   KEY_F4          ,   KEY_F4          },
    /*  63  */  {   KEY_F5          ,   KEY_F5          },
    /*  64  */  {   KEY_F6          ,   KEY_F6          },
    /*  65  */  {   KEY_F7          ,   KEY_F7          },
    /*  66  */  {   KEY_F8          ,   KEY_F8          },
    /*  67  */  {   KEY_F9          ,   KEY_F9          },
    /*  68  */  {   KEY_F10         ,   KEY_F10         },
    /*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
    /*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },

    /*  71  */  {   KEY_HOME        ,   '7'             },
    /*  72  */  {   KEY_UP          ,   '8'             },
    /*  73  */  {   KEY_PAGEUP      ,   '9'             },
    /*  74  */  {   '-'             ,   '-'             },
    /*  75  */  {   KEY_LEFT        ,   '4'             },
    /*  76  */  {   KEY_CENTER      ,   '5'             },
    /*  77  */  {   KEY_RIGHT       ,   '6'             },
    /*  78  */  {   '+'             ,   '+'             },
    /*  79  */  {   KEY_END         ,   '1'             },
    /*  80  */  {   KEY_DOWN        ,   '2'             },
    /*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
    /*  82  */  {   KEY_INS         ,   '0'             },
    /*  83  */  {   KEY_DEL         ,   '.'             },
    /*  84  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  85  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  86  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  87  */  {   KEY_F11         ,   KEY_F11         },
    /*  88  */  {   KEY_F12         ,   KEY_F12         }
};

bool kIsAlphabetScanCode(u8 bScanCode) {
    if(('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) &&
        (gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z'))
        return true;
    return false;
}

bool kIsNumberOrSymbolScanCode(u8 bScanCode) {
    if((2 <= bScanCode) && (bScanCode <= 53) && !kIsAlphabetScanCode(bScanCode))
        return true;
    return false;
}

bool kIsNumberPadScanCode(u8 bScanCode) {
    if((71 <= bScanCode) && (bScanCode <= 83)) return true;
    return false;
}

bool kIsUseCombinedCode(u8 bScanCode) {
    u8 bDownScanCode = bScanCode & 0x7F;
    bool bUseCombinedKey = false;

    if(kIsAlphabetScanCode(bDownScanCode))
        if(gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockDown)
            bUseCombinedKey = true;
    else if(kIsNumberOrSymbolScanCode(bDownScanCode))
        if(gs_stKeyboardManager.bShiftDown)
            bUseCombinedKey = true;
    else if(kIsNumberPadScanCode(bDownScanCode) &&
            !gs_stKeyboardManager.bExtendedCodeIn)
        if(gs_stKeyboardManager.bNumLockDown)
            bUseCombinedKey = true;
    
    return bUseCombinedKey;
}

void kUpdateCombinationKeyStatusAndLED(u8 bScanCode) {
    bool bDown;
    bool bLEDStatusChanged = false;
    u8 bDownScanCode;

    if(bScanCode & 0x80) {
        bDown = false;
        bDownScanCode = bScanCode & 0x7F;
    } else {
        bDown = true;
        bDownScanCode = bScanCode;
    }

    if((bDownScanCode == 42) || (bDownScanCode == 54))
        gs_stKeyboardManager.bShiftDown = bDown;
    else if((bDownScanCode == 58) && bDown) {
        gs_stKeyboardManager.bCapsLockDown = true;
        bLEDStatusChanged = true;
    }
    else if((bDownScanCode == 69) && bDown) {
        gs_stKeyboardManager.bCapsLockDown ^= true;
        bLEDStatusChanged = true;
    }
    else if((bDownScanCode == 70) && bDown) {
        gs_stKeyboardManager.bScrollLockDown ^= true;
        bLEDStatusChanged = true;
    }

    if(bLEDStatusChanged)
        kChangeKeyboardLED(
            gs_stKeyboardManager.bCapsLockDown,
            gs_stKeyboardManager.bNumLockDown,
            gs_stKeyboardManager.bScrollLockDown
        );
}

bool kConvertScanCodeToASCIICode(u8 bScanCode, u8 & bASCIICode, u8 & bFlags) {
    if(gs_stKeyboardManager.iSkipCountForPause > 0) {
        gs_stKeyboardManager.iSkipCountForPause--;
        return false;
    }
    if(bScanCode == 0xE1) {
        bASCIICode = KEY_PAUSE;
        bFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return true;
    }
    if(bScanCode == 0xE0) {
        gs_stKeyboardManager.bExtendedCodeIn = true;
        return false;
    }

    bool bUseCombinedKey = kIsUseCombinedCode(bScanCode);

    if(bUseCombinedKey) bASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
    else bASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;

    if(gs_stKeyboardManager.bExtendedCodeIn) {
        bFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = false;
    } else bFlags = 0;

    if((bScanCode & 0x80) == 0) bFlags |= KEY_FLAGS_DOWN;

    kUpdateCombinationKeyStatusAndLED(bScanCode);
    return true;
}