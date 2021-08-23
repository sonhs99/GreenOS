[BITS 64]

section .text
global kInPortByte, kOutPortByte, kLoadGDTR, kLoadTR, kLoadIDTR
global kEnableInterrupt, kDisableInterrupt, kReadFLAGS
global kReadTSC
global kSwitchContext

kInPortByte:
    push rdx
    
    mov rdx, rdi
    mov rax, 0
    in al, dx

    pop rdx
    ret

kOutPortByte:
    push rdx
    push rax

    mov rdx, rdi
    mov rax, rsi
    out dx, al

    pop rax
    pop rdx
    ret

kLoadGDTR:
    lgdt [rdi]
    ret

kLoadTR:
    ltr di
    ret

kLoadIDTR:
    lidt[rdi]
    ret

kEnableInterrupt:
    sti
    ret

kDisableInterrupt:
    cli
    ret

kReadFLAGS:
    pushfq
    pop rax

    ret

kReadTSC:
    push rdx

    rdtsc
    shl rdx, 32
    or rax, rdx

    pop rdx
    ret