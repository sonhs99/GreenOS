#pragma once

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long;

#define offsetof(TYPE, MEMBER) __builtin_offsetof(TYPE, MEMBER)

#pragma pack(push , 1)

struct Charactor {
    u8 bCharactor;
    u8 bAttribute;
};

#pragma pack(pop)

#define assert(X) if(!(X)) { while(true); }