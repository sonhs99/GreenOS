#include "Types.h"

void kPrintString(int iX, int iY, const char * pcString);
extern "C"{
    void Main() {
        kPrintString( 0, 10, "Switch To IA-32e Mode.......................[Pass]");
        kPrintString( 0, 11, "IA-32e C++ Language Kernel Start............[Pass]");
    }
}

void kPrintString(int iX, int iY, const char * pcString){
    Charactor* pstScreen = (Charactor*) 0xB8000;
    pstScreen += (iY * 80) + iX;
    for(int i = 0; pcString[i] != 0; i++)
        pstScreen[i].bCharactor = pcString[i];
}