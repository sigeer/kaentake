#include "pch.h"
#include "hook.h"
#include <imm.h>

#pragma comment(lib, "imm32.lib")

BYTE g_ImeEnabled = 1;

namespace FixIme {

// IME 启用函数地址
constexpr DWORD FUNC_ENABLE_IME_ADDR = 0x009E85F3;

// CCtrlEdit 焦点处理相关地址
constexpr DWORD SET_ON_FOCUS_JUDGEMENT_RTN_ADDR = 0x004CA061;
constexpr DWORD SWITCH_IME_ADDR = 0x004CA078;
constexpr DWORD ENABLE_RTN_ADDR = 0x004CA08F;
constexpr DWORD DISABLE_RTN_ADDR = 0x004CA091;

// CCtrlMLEdit 多行编辑框相关地址
constexpr DWORD SWITCH_ML_IME_ADDR = 0x004D32D9;
constexpr DWORD ENABLE_ML_RTN_ADDR = 0x004D32E0;
constexpr DWORD DISABLE_ML_RTN_ADDR = 0x004D32E2;

// 窗口销毁相关地址
constexpr DWORD DESTROY_WINDOW_RTN_ADDR = 0x004DFEAD;
constexpr DWORD DESTROY_WINDOW_FUNC_ADDR = 0x0041FE69;

// 新版 IME 切换返回地址
constexpr DWORD NEW_SWITCH_IME_RTN_ADDR = 0x004CA08F;
constexpr DWORD NEW_SWITCH_ML_IME_RTN_ADDR = 0x004D32EE;

// 通用 Hook：移除游戏对 IME 的限制
void GeneralHook() {
    // 聊天输入框的 IME 限制
    PatchNop(0x008D54A6, 0x008D54AF); // Key input
    PatchNop(0x00937225, 0x0093722E); // Chat
    PatchNop(0x00531EE8, 0x00531EF1); // Group Message
    
    // 编辑框支持
    PatchNop(0x004CAE7D, 0x004CAE7F); // CCtrlEdit
    Patch1(0x004CAE8F, 0xEB); // CCtrlEdit
    
    // 角色创建界面的 IME 支持
    PatchNop(0x007A015D, 0x007A015F);
}

// 启用 IME
void EnableIme() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        HIMC hImc = ImmGetContext(hwnd);
        if (hImc) {
            ImmAssociateContext(hwnd, hImc);
            ImmReleaseContext(hwnd, hImc);
        }
    }
}

// 禁用 IME
void DisableIme() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        HIMC hImc = ImmGetContext(hwnd);
        if (hImc) {
            ImmAssociateContext(hwnd, nullptr);
            ImmReleaseContext(hwnd, hImc);
        }
    }
}

// CCtrlEdit 焦点处理：跳转到 IME 切换逻辑
__declspec(naked) void SetOnFocusFirstJudgement() {
    __asm {
        cmp[esp + 0Ch], edi
        jz label_jmp_switch_ime
        jmp SET_ON_FOCUS_JUDGEMENT_RTN_ADDR
        
label_jmp_switch_ime:
        jmp SWITCH_IME_ADDR
    }
}

// CCtrlEdit IME 切换逻辑
__declspec(naked) void SwitchIme() {
    __asm {
        cmp[esp + 0Ch], edi
        jz label_jz
        xor eax, eax
        cmp[esi + 80h], eax
        setz al
        push eax
        call FUNC_ENABLE_IME_ADDR
        mov g_ImeEnabled, 1
        jmp ENABLE_RTN_ADDR

label_jz:
        push 0
        call FUNC_ENABLE_IME_ADDR
        jmp DISABLE_RTN_ADDR
    }
}

// CCtrlMLEdit IME 切换逻辑
__declspec(naked) void SwitchMLIme() {
    __asm {
        cmp dword ptr[esp + 8], 0
        jz label_jz
        push 1
        call FUNC_ENABLE_IME_ADDR
        mov g_ImeEnabled, 1
        jmp ENABLE_ML_RTN_ADDR

label_jz:
        push 0
        call FUNC_ENABLE_IME_ADDR
        jmp DISABLE_ML_RTN_ADDR
    }
}

// 新版 CCtrlEdit IME 切换逻辑（适用于 Win10/11）
__declspec(naked) void NewSwitchIme() {
    __asm {
        cmp[esi + 80h], 1
        jz label_disable
        push 1
        call FUNC_ENABLE_IME_ADDR
        mov g_ImeEnabled, 1
        jmp NEW_SWITCH_IME_RTN_ADDR

label_disable:
        call DisableIme
        jmp NEW_SWITCH_IME_RTN_ADDR
    }
}

// 窗口销毁时固定清除 IME 状态
__declspec(naked) void DestroyWindow() {
    __asm {
        call DESTROY_WINDOW_FUNC_ADDR
        or dword ptr[esi + 14h], 0FFFFFFFFh

        cmp g_ImeEnabled, 0
        jz label_return

        call DisableIme
        mov g_ImeEnabled, 0

label_return:
        jmp DESTROY_WINDOW_RTN_ADDR
    }
}

// 新版 CCtrlMLEdit IME 切换逻辑
__declspec(naked) void NewSwitchMLIme() {
    __asm {
        push 1
        call FUNC_ENABLE_IME_ADDR
        mov g_ImeEnabled, 1
        jmp NEW_SWITCH_ML_IME_RTN_ADDR
    }
}

// HookOld：适用于老版本 Windows 10 系统
void HookOld() {
    GeneralHook();
    
    // 修复 CCtrlEdit::OnSetFocus
    PatchJmp(0x004CA05B, &SetOnFocusFirstJudgement);
    PatchJmp(0x004CA089, &SwitchIme);
    
    // 修复 CCtrlMLEdit::OnSetFocus
    PatchNop(0x004D32C6, 0x004D32C8);
    PatchJmp(0x004D32D9, &SwitchMLIme);
    
    // 窗口销毁时固定清除 IME
    PatchJmp(0x004DFEA4, &DestroyWindow);
    
    DEBUG_MESSAGE("Old Ime Hook applied");
}

// HookNew：适用于新版本 Windows 10/11 系统
void HookNew() {
    GeneralHook();
    
    // 使用新版 IME 切换
    PatchJmp(0x004CA089, &NewSwitchIme);
    
    // 窗口销毁时固定清除 IME
    PatchJmp(0x004DFEA4, &DestroyWindow);
    
    // 使用新版多行编辑框 IME 切换
    PatchJmp(0x004D32D9, &NewSwitchMLIme);
    
    DEBUG_MESSAGE("New Ime Hook applied");
}

} // namespace FixIme


namespace FixBuddy {

// 好友申请修复相关地址
constexpr DWORD FIX_BUDDY_ACCEPT_FUNC = 0x00A3FA51;
constexpr DWORD FIX_BUDDY_ACCEPT_JMP = 0x005312D5;
constexpr DWORD FIX_BUDDY_ACCEPT_RTN = 0x005312A3;

// 好友申请修复逻辑
__declspec(naked) void FixBuddyAccept() {
    __asm {
        call FIX_BUDDY_ACCEPT_FUNC
        cmp byte ptr[ebp - 33h], 9
        jz label_jmp
        jmp FIX_BUDDY_ACCEPT_RTN

label_jmp:
        jmp FIX_BUDDY_ACCEPT_JMP
    }
}

// Hook：应用好友申请修复
void Hook() {
    PatchJmp(0x0053129E, &FixBuddyAccept);
    DEBUG_MESSAGE("FixBuddy Hook applied");
}

} // namespace FixBuddy


namespace Ime {

// IME 类型：0=旧版Hook, 1=新版Hook
static unsigned char g_ImeType = 1;

// 设置 IME 类型
void SetImeType(unsigned char type) {
    g_ImeType = type;
}

// 获取当前 IME 状态
bool IsImeEnabled() {
    return g_ImeEnabled != 0;
}

// 获取当前 IME 类型
unsigned char GetImeType() {
    return g_ImeType;
}

// SwitchChinese=true 时的特定修改
void ApplyChineseModeFixes() {
    DEBUG_MESSAGE("Applying Chinese mode fixes...");

    // 窗口标题修改
    PatchStr(0x00AF2B28, "对联盟      ");

    // 字体大小 sub_46341A 第2个参数
    // 特效字体大小修改
    Patch1(0x008E55ED + 1, 0x0B);

    // 名称字体大小修改
    Patch1(0x008E557A + 1, 0x0B);
    Patch1(0x008E565E + 1, 0x0B);

    // 技能片/职业名称大小和位置修改
    Patch1(0x0090142E + 1, 0x5E); // 60->5E 位置调整
    Patch1(0x00901400 + 1, 1);    // 类型设为1 对应12号大小

    DEBUG_MESSAGE("Chinese mode fixes applied");
}

} // namespace Ime

// 初始化 IME 支持
void AttachImeSupport() {
    DEBUG_MESSAGE("Attaching IME support...");

    // 根据 IME 类型选择合适的 Hook
    if (Ime::GetImeType() == 0) {
        FixIme::HookOld();
    } else {
        FixIme::HookNew();
    }

    // 应用好友申请修复
    // FixBuddy::Hook();
    Ime::ApplyChineseModeFixes();

    DEBUG_MESSAGE("IME support attached successfully");
}