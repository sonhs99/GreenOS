#pragma once

#include "Types.hpp"

#pragma pack(push, 1)

struct ListNode {
    ListNode *pstNext;
    u64 qwID;
};

class List {
    ListNode *pstHead, *pstTail;
    int iItemCount;
public:
    List(): pstHead(nullptr), pstTail(nullptr), iItemCount(0) {}

    int ItemCount() { return iItemCount; }
    ListNode* Head() { return pstHead; }
    ListNode* Tail() { return pstTail; }
    ListNode* Next(ListNode* Curr) { return Curr->pstNext; }

    void AddListToTail(ListNode* tItem);
    void AddListToHeader(ListNode* tItem);
    ListNode* Remove(u64 qwID);
    ListNode* Search(u64 qwID);

    ListNode* RemoveListFromHead() {
        if(iItemCount == 0) return nullptr;
		return Remove(pstHead->qwID);
    }

    ListNode* RemoveListFromTail() {
        if(iItemCount == 0) return nullptr;
        return Remove(pstTail->qwID);
    }
};

#pragma pack(pop)