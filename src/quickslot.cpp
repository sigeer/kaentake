#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// QuickSlot - 长快捷栏（4 -> 26 个槽）。从
// BeiDou-ijl15/ezorsia/Client.cpp::LongQuickSlot 移植。
// ============================================================================

void QuickSlot() {
    // CUIStatusBar::OnCreate - 扩展快捷栏绘制
    Patch1(0x008D155C + 1, 0xF0);
    Patch1(0x008D155C + 2, 0x03);
    Patch1(0x008D182E + 1, 0xF0);
    Patch1(0x008D182E + 2, 0x03);
    Patch1(0x008D1AC0 + 1, 0xF0);
    Patch1(0x008D1AC0 + 2, 0x03);

    Patch4(0x0072B7CE + 1, (DWORD)Array_aDefaultQKM_0);
    Patch4(0x0072B8EB + 1, (DWORD)Array_aDefaultQKM_0);

    // CUIStatusBar::CQuickSlot::CompareValidateFuncKeyMappedInfo
    Patch1(0x008DD916, 0x1A);
    Patch1(0x008DD8AD, 0x1A);
    Patch1(0x008DD8FD, 0xBB);
    Patch4(0x008DD8FD + 1, (DWORD)Array_Expanded);
    Patch1(0x008DD8FD + 5, 0x90);
    Patch1(0x008DD898, 0xB8);
    Patch4(0x008DD898 + 1, (DWORD)Array_Expanded);
    Patch1(0x008DD898 + 5, 0x90);

    // CUIStatusBar::CQuickSlot::Draw
    Patch1(0x008DE75E + 3, 0x6C);
    Patch1(0x008DDF99, 0xB8);
    Patch4(0x008DDF99 + 1, (DWORD)Array_Expanded);
    unsigned char nops3[3] = { 0x90, 0x90, 0x90 };
    PatchMemory((void*)0x008DDF99 + 5, nops3, 3);

    // CUIStatusBar::OnMouseMove
    Patch1(0x008D7F1E + 1, 0x34);
    Patch1(0x008D7F1E + 2, 0x85);
    Patch4(0x008D7F1E + 3, (DWORD)Array_Expanded);

    // CUIStatusBar::CQuickSlot::GetPosByIndex
    Patch4(0x008DE94D + 2, (DWORD)Array_ptShortKeyPos);
    Patch4(0x008DE955 + 2, (DWORD)Array_ptShortKeyPos + 4);
    Patch1(0x008DE941 + 2, 0x1A);

    // CUIStatusBar::GetShortCutIndexByPos
    Patch4(0x008DE8F4 + 1, (DWORD)Array_ptShortKeyPos_Fixed_Tooltips + 4);
    Patch1(0x008DE926 + 1, 0x3E);

    // CUIStatusBar::CQuickSlot::DrawSkillCooltime
    Patch1(0x008E099F + 3, 0x1A);
    Patch1(0x008E069D, 0xBE);
    Patch4(0x008E069D + 1, (DWORD)cooldown_Array);
    Patch1(0x008E069D + 5, 0x90);
    Patch1(0x008E06A3, 0xBF);
    Patch4(0x008E06A3 + 1, (DWORD)Array_Expanded + 1);
    Patch1(0x008E06A3 + 5, 0x90);

    // CDraggableMenu::OnDropped
    Patch1(0x004F928A + 2, 0x1A);
    // CDraggableMenu::MapFuncKey
    Patch1(0x004F93F9 + 2, 0x1A);
    // CUIKeyConfig::OnDestroy
    Patch1(0x00833797 + 2, 0x6C);
    Patch1(0x00833841 + 2, 0x6C);
    Patch1(0x00833791 + 1, 0x68);
    Patch1(0x0083383B + 1, 0x68);
    // CUIKeyConfig::~CUIKeyConfig
    Patch1(0x0083287F + 2, 0x6C);
    Patch1(0x00832882 + 1, 0x68);
    // CQuickslotKeyMappedMan::SaveQuickslotKeyMap
    Patch1(0x0072B8C0 + 2, 0x6C);
    Patch1(0x0072B8A0 + 1, 0x68);
    Patch1(0x0072B8BD + 1, 0x68);
    // CQuickslotKeyMappedMan::OnInit
    Patch1(0x0072B861 + 1, 0x68);
    Patch1(0x0072B867 + 2, 0x6C);
    // CUIKeyConfig::CNoticeDlg::OnChildNotify
    Patch1(0x00836A1E + 1, 0x68);
    Patch1(0x00836A21 + 2, 0x6C);

    CodeCave((void*)0x8DD8B8,    (void*)CompareValidateFuncKeyMappedInfo_cave, 5);
    CodeCave((void*)0x9FA0DB,    (void*)sub_9FA0CB_cave, 5);
    CodeCave((void*)0x72B7BC,    (void*)sDefaultQuickslotKeyMap_cave, 5);
    CodeCave((void*)0x72B8E6,    (void*)DefaultQuickslotKeyMap_cave, 5);
    CodeCave((void*)0x008CFDFD,  (void*)Restore_Array_Expanded, 6);
}


void AttachQuickSlotMod() {
    QuickSlot();
}
