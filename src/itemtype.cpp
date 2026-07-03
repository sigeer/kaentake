#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// ItemType - 绕过装备类型查询中的位数上限，使 5 位数装备类型可被读取。
// 从 BeiDou-ijl15/ezorsia/Client.cpp::FixItemType 移植。
// ============================================================================

void ItemType() {
    CodeCave((void*)0x005CFA99, (void*)getItemType1, 15);
    CodeCave((void*)getItemType2Addr, (void*)getItemType2, 27);
}


void AttachItemTypeMod() {
    ItemType();
}
