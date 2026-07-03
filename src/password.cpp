#include "pch.h"
#include "hook.h"


// ============================================================================
// Password - 当启用 noPassword 时跳过二级密码对话框。
// 从 BeiDou-ijl15/ezorsia/Client.cpp::NoPassword 移植。
// ============================================================================

extern bool noPassword;


void Password() {
    if (noPassword) {
        Patch4(0x00620F2F + 2, 0);
    }
}


void AttachPasswordMod() {
    Password();
}
