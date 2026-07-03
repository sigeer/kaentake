#include "pch.h"
#include "hook.h"
#include <imm.h>
#pragma comment(lib, "imm32.lib")


// ============================================================================
// FixIme - 聊天输入框的 IME（输入法编辑器）修复。
// 从 BeiDou-ijl15/ezorsia/FixIme.h 移植。
// ============================================================================


class FixIme {
public:
    static void HookOld();
    static void HookNew();
};


static void EnableIme() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        HIMC hImc = ImmGetContext(hwnd);
        if (hImc) {
            ImmAssociateContext(hwnd, hImc);
            ImmReleaseContext(hwnd, hImc);
        }
    }
}

static void DisableIme() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        HIMC hImc = ImmGetContext(hwnd);
        if (hImc) {
            ImmAssociateContext(hwnd, NULL);
            ImmReleaseContext(hwnd, hImc);
        }
    }
}


static BYTE g_imeEnabled = 1;
static const DWORD g_funcEnableImeAddr = 0x009E85F3;
static const DWORD g_destroyWindowFuncAddr = 0x0041FE69;


// ----- 代码洞穴 -----
// [推理] 拦截 CCtrlEdit::OnSetFocus（约 0x004CA05B）：先判断参数 [esp+0Ch]
// 是否等于某个标志 edi；若是则跳到 switchIme 处理 IME 切换，否则跳回原流程。
static const DWORD setOnFocusFirstJudgementRtnAddr = 0x004CA061;
static const DWORD switchImeAddr = 0x004CA078;
__declspec(naked) void setOnFocusFirstJudgement() {
    __asm {
        cmp[esp + 0Ch], edi
        jz label_jmp_switch_ime
        jmp setOnFocusFirstJudgementRtnAddr
    label_jmp_switch_ime :
        jmp switchImeAddr
    }
}

// [推理] 旧版 IME 切换逻辑：依据 [esi+0x80] 的当前状态调用 g_funcEnableImeAddr
// 启用或禁用 IME，并把 g_imeEnabled 标志置 1/0。
static const DWORD enableRtnAddr = 0x004CA08F;
static const DWORD disableRtnAddr = 0x004CA091;
__declspec(naked) void switchIme() {
    __asm {
        cmp [esp + 0Ch], edi
        jz  label_jz
        xor eax, eax
        cmp [esi + 0x80], eax
        setz al
        push eax
        call g_funcEnableImeAddr
        mov g_imeEnabled, 1
        jmp  enableRtnAddr
    label_jz :
        push 0
        call g_funcEnableImeAddr
        jmp  disableRtnAddr
    }
}

// [推理] 多行编辑（CCtrlMLEdit）的旧版 IME 切换：参数 [esp+8]==0 时禁用，
// 否则启用；与 switchIme 行为类似但作用于 MLEdit 控件。
static const DWORD enableMLRtnAddr = 0x004D32E0;
static const DWORD disableMLRtnAddr = 0x004D32E2;
__declspec(naked) void switchMLIme() {
    __asm {
        cmp  dword ptr[esp + 8], 0
        jz   label_jz
        push 1
        call g_funcEnableImeAddr
        mov g_imeEnabled, 1
        jmp  enableMLRtnAddr
    label_jz :
        push 0
        call g_funcEnableImeAddr
        jmp  disableMLRtnAddr
    }
}

// [推理] 新版 IME 切换逻辑：若 [esi+0x80]==1 则调用 DisableIme，
// 否则启用 IME；比旧版更直接，避免重复启用同一上下文。
static const DWORD newSwitchImeRtnAddr = 0x004CA08F;
__declspec(naked) void newSwitchIme() {
    __asm {
        cmp[esi + 0x80], 1
        jz label_disable
        push 1
        call g_funcEnableImeAddr
        mov g_imeEnabled, 1
        jmp newSwitchImeRtnAddr
    label_disable :
        call DisableIme
        jmp newSwitchImeRtnAddr
    }
}

// [推理] 在窗口销毁时清理 IME：调用 g_destroyWindowFuncAddr 后，
// 若 g_imeEnabled==1 则调用 DisableIme 释放 IME 上下文。
static const DWORD destroyWindowRtnAddr = 0x004DFEAD;
__declspec(naked) void destroyWindow() {
    __asm {
        call g_destroyWindowFuncAddr
        or dword ptr[esi + 14h], 0FFFFFFFFh
        cmp g_imeEnabled, 0
        jz label_return
        call DisableIme
        mov g_imeEnabled, 0
    label_return :
        jmp destroyWindowRtnAddr
    }
}

// [推理] 新版多行编辑的 IME 切换：始终启用 IME。
static const DWORD newSwitchMLImeRtnAddr = 0x004D32EE;
__declspec(naked) void newSwitchMLIme() {
    __asm {
        push 1
        call g_funcEnableImeAddr
        mov g_imeEnabled, 1
        jmp  newSwitchMLImeRtnAddr
    }
}


static void GeneralHook() {
    unsigned char nops9[9] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    PatchMemory((void*)0x008D54A6, nops9, 9);
    PatchMemory((void*)0x00937225, nops9, 9);
    PatchMemory((void*)0x00531EE8, nops9, 9);
    unsigned char nops2[2] = { 0x90, 0x90 };
    PatchMemory((void*)0x004CAE7D, nops2, 2);
    Patch1(0x004CAE8F, 0xEB);
    PatchMemory((void*)0x007A015D, nops2, 2);
}


void FixIme::HookOld() {
    GeneralHook();
    // CCtrlEdit::OnSetFocus
    CodeCave((void*)0x004CA05B, (void*)setOnFocusFirstJudgement, 6);
    CodeCave((void*)0x004CA089, (void*)switchIme, 6);
    // CCtrlMLEdit::OnSetFocus
    unsigned char nops2[2] = { 0x90, 0x90 };
    PatchMemory((void*)0x004D32C6, nops2, 2);
    CodeCave((void*)0x004D32D9, (void*)switchMLIme, 7);
    CodeCave((void*)0x004DFEA4, (void*)destroyWindow, 9);
    DEBUG_MESSAGE("Old Ime Hook");
}

void FixIme::HookNew() {
    GeneralHook();
    CodeCave((void*)0x004CA089, (void*)newSwitchIme, 6);
    CodeCave((void*)0x004DFEA4, (void*)destroyWindow, 9);
    CodeCave((void*)0x004D32D9, (void*)newSwitchMLIme, 7);
    DEBUG_MESSAGE("New Ime Hook");
}


void AttachImeFixMod() {
    // BeiDou-ijl15 原本会检测 Windows 版本；这里默认使用 New。
    // 如需切换到 HookOld()，修改这一行即可。
    FixIme::HookNew();
}
