#include "Types.hpp"
#include "Page.hpp"
#include "ModeSwitch.hpp"

void kPrintString(int iX, int iY, const char * pcString);
bool kInitializeKernel64Area();
bool kIsMemoryEnough();
void kCopyKernel64ImageTo2Mbyte();

void Main( void ){
    u32 dwEAX, dwEBX, dwECX, dwEDX;
    char vcVendorString[ 13 ] = "";

    kPrintString( 0, 3, "Protected Mode C++ Language Kernel Start....[Pass]");
    kPrintString( 0, 4, "Minimum Memory Size Check...................[    ]");
    if( !kIsMemoryEnough() ) {
        kPrintString(45, 4, "Fail");
        kPrintString( 0, 5, "[ERROR] Not Enough Memory. Require Over 64MB Memory");
        while(1);
    } else kPrintString(45, 4, "Pass");

    kPrintString( 0, 5, "IA-32e Kernel Area Initialize...............[    ]");
    if( !kInitializeKernel64Area() ) {
        kPrintString(45, 5, "Fail");
        kPrintString( 0, 6, "[ERROR] Kernel Area Initialization Failed");
        while(1);
    } else kPrintString(45, 5, "Pass");

    kPrintString( 0, 6, "IA-32e Page Table Initialize................[    ]");
    kInitializePageTables();
    kPrintString(45, 6, "Pass");

    kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    ((u32*) vcVendorString)[0] = dwEBX;
    ((u32*) vcVendorString)[1] = dwEDX;
    ((u32*) vcVendorString)[2] = dwECX;
    kPrintString( 0, 7, "Processor Vendor String.....................[            ]");
    kPrintString(45, 7, vcVendorString);

    kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    kPrintString( 0, 8, "64bit Mode Support Check....................[    ]");
    if( dwEDX & ( 1 << 29 )) {
        kPrintString(45, 8, "Pass");
    } else {
        kPrintString(45, 8, "Fail");
        kPrintString( 0, 9, "[ERROR] This Processor does not support 64bit mode");
        while(1);
    }

    kPrintString( 0, 9, "Copy IA-32e Kernel To 2M Address............[    ]");
    kCopyKernel64ImageTo2Mbyte();
    kPrintString( 45, 9, "Pass");

    kPrintString( 0, 10, "Switch To IA-32e Mode");
    kSwitchAndExecute64BitKernel();
    while(1);
}

void kPrintString(int iX, int iY, const char * pcString){
    Charactor* pstScreen = (Charactor*) 0xB8000;
    pstScreen += (iY * 80) + iX;
    for(int i = 0; pcString[i] != 0; i++)
        pstScreen[i].bCharactor = pcString[i];
}

bool kInitializeKernel64Area() {
    u32* pdwCurrentAddress = (u32*) 0x100000;
    while( u32(pdwCurrentAddress) < 0x600000 ){
        *pdwCurrentAddress = 0x00;
        if(*pdwCurrentAddress != 0) return false;
        pdwCurrentAddress++;
    }
    return true;
}

bool kIsMemoryEnough() {
    u32* pdwCurrentAddress = (u32*) 0x100000;
    while( u32(pdwCurrentAddress) < 0x4000000 ){
        *pdwCurrentAddress = 0x12345678;
        if(*pdwCurrentAddress != 0x12345678) return false;
        pdwCurrentAddress += (0x100000 / 4);
    }
    return true;
}

void kCopyKernel64ImageTo2Mbyte() {
    u16 wKernel32SectorCount = *((u16*) 0x7C07);
    u16 wTotalKernelSectorCount = *((u16*) 0x7C05);
    u32* pdwSourceAddress = (u32*) (0x10000 + (wKernel32SectorCount * 512));
    u32* pdwDestinationAddress = (u32*) 0x200000;

    for(int i = 0; i < 512 * (wTotalKernelSectorCount - wKernel32SectorCount) / 4; i++){
        *pdwDestinationAddress = *pdwSourceAddress;
        pdwDestinationAddress++;
        pdwSourceAddress++;
    }

}