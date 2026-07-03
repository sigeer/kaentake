#include "pch.h"
#include "hook.h"


// ============================================================================
// FixBuddy - 当接收方处于免打扰状态时自动接受好友请求。
// 从 BeiDou-ijl15/ezorsia/FixBuddy.h 移植。
// ============================================================================


class FixBuddy {
public:
    static void Hook();
};


static const DWORD fixBuddyAcceptFunc = 0x00A3FA51;
static const DWORD fixBuddyAcceptJmp = 0x005312D5;
static const DWORD fixBuddyAcceptRtn = 0x005312A3;
// [推理] 拦截 CField::SendAcceptFriendMsg：先调 sub_A3FA51（CFriend::FindIndex
// 拷贝好友信息），若对方状态字段 [ebp-33h]==9（屏蔽 / 免打扰），
// 直接走 loc_5312D5 发送接受封包，绕过弹窗；否则回退到原流程。
__declspec(naked) void fixBuddyAccept() {
    __asm {
        call fixBuddyAcceptFunc
        cmp byte ptr[ebp - 33h], 9
        jz label_jmp
        jmp fixBuddyAcceptRtn
    label_jmp :
        jmp fixBuddyAcceptJmp
    }
}


void FixBuddy::Hook() {
    CodeCave((void*)0x0053129E, (void*)fixBuddyAccept, 5);
}


void AttachBuddyFixMod() {
    FixBuddy::Hook();
}
