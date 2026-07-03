#include "pch.h"
#include "hook.h"
#include "codecaves.h"
#include "constants.h"


// ============================================================================
// ClientStartup - 在 DLL 加载时应用的所有一次性内存补丁。
// 同时定义供其他功能模块（jumpcap.cpp、extras.cpp、password.cpp 等）
// 以及 codecaves.h 中的内联汇编块使用的共享配置变量。
// ============================================================================

// ----- 共享配置变量（在 codecaves.h 中通过 extern 声明） -----
std::string ServerIP_AddressFromINI = CONSTANTS_DEFAULT_HOST;
int  serverIP_Port = 0;

int  setDamageCap = 199999;
int  setMAtkCap = 1999;
int  setAccCap = 999;
int  setAvdCap = 999;
double setAtkOutCap = 199999.0;

int  speedMovementCap = 140;
DWORD jumpCap = 123;
float climbSpeed = 1.0f;
bool  climbSpeedAuto = false;

bool useTubi = false;
bool noPassword = false;
bool talkRepeat = false;
int  talkTime = 2000;


void ClientStartup() {
    // 去除提权请求
    PatchMemory((void*)0x00C08459, "\x22", 1);

    // 用户配置的服务器 IP 与端口
    const char* serverIP_Address = ServerIP_AddressFromINI.c_str();
    if (serverIP_Address) {
        PatchStr(0x00AFE084, serverIP_Address);
        PatchStr(0x00AFE084 + 16, serverIP_Address);
        PatchStr(0x00AFE084 + 32, serverIP_Address);
    }
    if (serverIP_Port) {
        Patch4(0x007519C1 + 1, serverIP_Port);
    }

    // Tubi：跳过 1 秒的属性点分配 / 宠物拾取卡顿
    if (useTubi) {
        unsigned char nops[2] = { 0x90, 0x90 };
        PatchMemory((void*)0x00485C32, nops, 2);
    }

    // 属性 / 伤害 / 命中上限
    Patch4(0x0077E055 + 1, 2147483646); // PAD 最小伤害上限
    Patch4(0x0077E12F + 1, 2147483646); // PAD 最大伤害上限
    Patch4(0x008C3304 + 1, setDamageCap);
    Patch4(0x0077E215 + 1, setMAtkCap);
    Patch4(0x00780620 + 1, setMAtkCap);
    Patch4(0x007806D0 + 1, setAccCap);
    Patch4(0x00780702 + 1, setAvdCap);
    Patch4(0x0078FF5F + 1, 2147483646); // CalcDamage::PDamage 999
    Patch4(0x0079166C + 1, 2147483646); // CalcDamage::MDamage 1999
    Patch4(0x00791CD5 + 1, 2147483646); // CalcDamage::MDamage 1999
    Patch4(0x0078E061 + 1, 2147483646);
    Patch4(0x0078E67D + 1, 2147483646);
    Patch4(0x007918FC + 1, 2147483646);

    *(double*)0x00AFE8A0 = setAtkOutCap;

    Patch4(0x00780743 + 3, speedMovementCap);
    Patch4(0x008C4286 + 1, speedMovementCap);
    Patch4(0x0094D91E + 1, speedMovementCap);

    // 中段换行修复 + 技能悬浮提示 CJK 宽度修复
    unsigned char nops2[2] = { 0x90, 0x90 };
    PatchMemory((void*)0x008E4252, nops2, 2);
    CodeCave((void*)0x008F383E, (void*)skillToolTipNew, 6);

    // 右下角消息计数器
    Patch1(0x0068DE1F + 1, 0x86);
    Patch1(0x0068DFBD + 1, 0x86);
    Patch1(0x0068E0E7 + 1, 0x86);
    Patch1(0x0068E534 + 1, 0x86);
    Patch1(0x0068E65D + 1, 0x86);
    Patch1(0x0068E709 + 1, 0x86);
}


void AttachClientStartupMod() {
    ClientStartup();
}
