#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// ChatPos - 修正聊天输入框位置，使光标在键入时保持在原位。
// 从 BeiDou-ijl15/ezorsia/Client.cpp::FixChatPosHook 移植。
// ============================================================================

void ChatPos() {
    CodeCave((void*)0x008DD06F, (void*)chatTextPos, 6);
}


void AttachChatPosMod() {
    ChatPos();
}
