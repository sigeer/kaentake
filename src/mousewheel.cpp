#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// MouseWheel - 将 UI 滚轮与鼠标位移解耦。从
// BeiDou-ijl15/ezorsia/Client.cpp::FixMouseWheel 移植。
// ============================================================================

void MouseWheel() {
    CodeCave((void*)0x009E8090, (void*)fixMouseWheelHook, 5);
}


void AttachMouseWheelMod() {
    MouseWheel();
}
