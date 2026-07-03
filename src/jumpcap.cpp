#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// JumpCap - 限制跳跃 / 移动 / 攀爬速度。从
// BeiDou-ijl15/ezorsia/Client.cpp::JumpCap 移植。
// 配置变量（jumpCap / speedMovementCap / climbSpeed）定义在 clientstartup.cpp 中，
// 并在 codecaves.h 的裸汇编里以名称引用。
// ============================================================================

extern DWORD jumpCap;
extern int   speedMovementCap;
extern float climbSpeed;
extern bool  climbSpeedAuto;


void JumpCap() {
    CodeCave((void*)0x00780797, (void*)customJumpCapHook1, 10);
    CodeCave((void*)0x008C42A3, (void*)customJumpCapHook2, 10);
    CodeCave((void*)0x0094D942, (void*)customJumpCapHook3, 5);

    Patch4(0x009CC6F9 + 2, 0x00C1CF80);
    if (climbSpeedAuto) {
        CodeCave((void*)0x0094D93C, (void*)calcSpeedHook, 6);
    } else {
        *(double*)0x00C1CF80 = climbSpeed * 3.0;
    }
}


void AttachJumpCapMod() {
    JumpCap();
}
