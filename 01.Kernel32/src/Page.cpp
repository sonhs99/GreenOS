#include "Page.hpp"

void kInitializePageTables() {
    PML4TENTRY* pstPML4TEntry = (PML4TENTRY*) 0x100000;
    pstPML4TEntry[0].set(0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
    for(int i = 1; i < PAGE_MAXENTRYCOUNT; i++)
        pstPML4TEntry[i].set(0, 0, 0, 0);

    PDPTENTRY* pstPDPTEntry = (PDPTENTRY*) 0x101000;
    for(int i = 0 ; i < 64; i++)
        pstPDPTEntry[i].set(0, 0x102000 + (i * PAGE_TABLESIZE), PAGE_FLAGS_DEFAULT, 0);
    for(int i = 64; i < PAGE_MAXENTRYCOUNT; i++)
        pstPDPTEntry[i].set(0, 0, 0, 0);
    
    PDENTRY* pstPDEntry = (PDENTRY*) 0x102000;
    u32 dwMappingAddress = 0;
    for(int i = 0; i < PAGE_MAXENTRYCOUNT * 64; i++){
        pstPDEntry[i].set((i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
        dwMappingAddress += PAGE_DEFAULTSIZE;
    }
}