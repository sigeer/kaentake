#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// Extras - 从 BeiDou-ijl15/ezorsia/Client.cpp 移植的杂项调整：
//   * MoreHook  - AP 详情按钮、对话气泡显示时长调整、装备外观宽度
//                 以及脸型 / 发型渲染修复
// ============================================================================

extern bool  talkRepeat;
extern int   talkTime;
extern double setAtkOutCap;


void Extras() {
    Patch4(0x009A3D81, 480);
    // 装备外观类名偏移
    Patch1(0x008EC4A7 + 1, 0x23);
    Patch1(0x008EC53C + 1, 0x4D);
    Patch1(0x008EC5D1 + 1, 0x7A);
    Patch1(0x008EC660 + 1, 0xA9);
    Patch1(0x008EC6CF + 1, 0xC8);

    CodeCave((void*)0x005C94F3, (void*)faceHairCave,       18);
    CodeCave((void*)0x00485C28, (void*)canSendPkgTimeCave, 10);

    if (talkRepeat) {
        Patch1(0x004905ED + 1, 5);
    }
    Patch4(0x0049064B + 2, talkTime);

    if (setAtkOutCap > 999999) {
        Patch4(0x008C485A + 1, 192);
        Patch4(0x008C4AB3 + 1, 210);
        Patch4(0x008C510A + 1, 218);
        Patch4(0x008C4EA2 + 1, 210);
        Patch4(0x008C5760 + 1, 210);
        Patch4(0x008C7AD9 + 1, 185);
        Patch4(0x008C2754 + 1, 195);
        Patch4(0x008C6C72 + 1, 210);
        CodeCave((void*)0x008C4E1B, (void*)apDetailBtn, 7);
    }

    // 装备外观宽度
    Patch4(0x0045A5BE + 1, 9999);
}


void AttachExtrasMod() {
    Extras();
}
