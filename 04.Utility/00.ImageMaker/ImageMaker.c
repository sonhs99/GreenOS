#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BYTEOFSECTOR 512

int CopyFile(int, int);
int AdjustInSectorSize(int, int);
void WriteKernelInformation(int, int, int);

int main(int argc, char * argv[]) {
    int iSourceFd;
    int iTargetFd;
    int iBootLoaderSize;
    int iKernel32SectorSize;
    int iKernel64SectorSize;
    int iSourceSize;

    if(argc < 3) {
        fprintf(stderr, "[ERROR] ImageMaker BootLoader Kernels\n");
        exit(-1);
    }

    if((iTargetFd = open("Disk.img", O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1){
        fprintf(stderr, "[ERROR] Cannot Open Disk.img\n");
        exit(-1);
    }

    if((iSourceFd = open(argv[1], O_RDONLY)) == -1){
        fprintf(stderr, "[ERROR] Cannot Open %s\n", argv[1]);
        exit(-1);
    }
    iSourceSize = CopyFile(iSourceFd, iTargetFd);
    iBootLoaderSize = AdjustInSectorSize(iTargetFd, iSourceSize);
    printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[1], iSourceSize, iBootLoaderSize);
    close(iSourceFd);

    if((iSourceFd = open(argv[2], O_RDONLY)) == -1){
        fprintf(stderr, "[ERROR] Cannot Open %s\n", argv[2]);
        exit(-1);
    }
    iSourceSize = CopyFile(iSourceFd, iTargetFd);
    iKernel32SectorSize = AdjustInSectorSize(iTargetFd, iSourceSize);
    printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[2], iSourceSize, iKernel32SectorSize);
    close(iSourceFd);

    if((iSourceFd = open(argv[3], O_RDONLY)) == -1){
        fprintf(stderr, "[ERROR] Cannot Open %s\n", argv[3]);
        exit(-1);
    }
    iSourceSize = CopyFile(iSourceFd, iTargetFd);
    iKernel64SectorSize = AdjustInSectorSize(iTargetFd, iSourceSize);
    printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[2], iSourceSize, iKernel32SectorSize);
    close(iSourceFd);

    printf("[INFO] Start to write kernel Information\n");
    WriteKernelInformation(iTargetFd, iKernel32SectorSize + iKernel64SectorSize, iKernel32SectorSize);
    printf("[INFO] Image file create complete\n");

    close(iTargetFd);
    return 0;
}

int CopyFile(int iSourceFd, int iTargetFd){
    int iSourceSize;
    int iRead;
    int iWrite;
    char vcBuffer[BYTEOFSECTOR];

    iSourceSize = 0;
    while(1) {
        iRead = read(iSourceFd, vcBuffer, sizeof(vcBuffer));
        iWrite = write(iTargetFd, vcBuffer, iRead);
        if(iRead != iWrite) {
            fprintf(stderr, "[ERROR] iWrite != iRead");
            exit(-1);
        }
        iSourceSize += iRead;
        if(iRead != BYTEOFSECTOR)
            break;
    }
    return iSourceSize;
}

int AdjustInSectorSize(int iFd, int iSourceSize){
    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTEOFSECTOR;
    cCh = 0x00;
    
    if(iAdjustSizeToSector != 0) {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        printf("[INFO] File Size = [%lu] and fill [%d] byte\n", iSourceSize, iAdjustSizeToSector);
        for(i = 0; i < iAdjustSizeToSector; i++) 
            write(iFd, &cCh, 1);
    } else {
        printf("[INFO] File size is aligned 512 byte\n");
    }
    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTEOFSECTOR;
    return iSectorCount;
}

void WriteKernelInformation(int iTargetFd, int iKernelSectorCount, int iKernel32SectorCount) {
    unsigned short usData;
    long lPosition;

    lPosition = lseek(iTargetFd, 5, SEEK_SET);
    if(lPosition == -1) {
        fprintf(stderr, "[ERROR] lseek fail\n");
        exit(-1);
    }

    usData = (unsigned short) iKernelSectorCount;
    write(iTargetFd, &usData, 2);
    usData = (unsigned short) iKernel32SectorCount;
    write(iTargetFd, &usData, 2);

    printf("[INFO] Total sector count except bootloader [%d]\n", iKernelSectorCount);
    printf("[INFO] Total sector count protected mode kernel [%d]\n", iKernel32SectorCount);    
}