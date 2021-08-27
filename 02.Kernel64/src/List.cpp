#include "List.hpp"
#include "Console.hpp"

void List::AddListToTail(ListNode* tItem) {
    tItem->pstNext = nullptr;
    if(pstHead != nullptr) pstTail->pstNext = tItem;
    else pstHead = tItem;
    pstTail = tItem;
    iItemCount++;
}

void List::AddListToHeader(ListNode* tItem) {
    tItem->pstNext = pstHead;
    if(pstHead == nullptr) pstTail = tItem;
    pstHead = tItem;
    iItemCount++;
}

ListNode* List::Remove(u64 qwID) {
    ListNode* prev = pstHead;
    for(ListNode* curr = pstHead; curr != nullptr; curr = curr->pstNext){
        if(curr->qwID == qwID) {
            if(pstHead == curr && pstTail == curr)
                pstHead = pstTail = nullptr;
            else if(curr == pstHead)
                pstHead = curr->pstNext;
            else if(curr == pstTail)
                pstTail = prev;
            else 
                prev->pstNext = curr->pstNext;
                
            iItemCount--;
            return curr;
        }
        prev = curr;
    }
    return nullptr;
}

ListNode* List::Search(u64 qwID) {
    for(ListNode* curr = pstHead; curr != nullptr; curr = curr->pstNext)
        if(curr->qwID == qwID) return curr;
    return nullptr;
}