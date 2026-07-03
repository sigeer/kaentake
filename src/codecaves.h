#pragma once
#include <windows.h>
#include <string>

// 在 clientstartup.cpp 中定义的外部配置变量。
// 下方的裸汇编代码块会按名称引用这些变量。
extern int  setDamageCap;
extern int  setMAtkCap;
extern int  setAccCap;
extern int  setAvdCap;
extern double setAtkOutCap;
extern int  speedMovementCap;
extern DWORD jumpCap;
extern float climbSpeed;
extern bool climbSpeedAuto;
extern bool useTubi;
extern bool noPassword;
extern bool talkRepeat;
extern int  talkTime;

// ============================================================================
// 内联汇编代码洞穴与数据数组（从 BeiDou-ijl15/ezorsia 移植）
// 与分辨率相关的代码洞穴已刻意排除。
// 注释以 [推理] 标记的，是通过反汇编 hook 目标地址上下文得出的功能说明。
// ============================================================================

// ----- 鼠标滚轮修复 -----
// 原始游戏会把滚轮消息误判为鼠标坐标变化（CWndMan::TranslateMessage），
// 通过本 cave 直接把滚轮消息（event==522）转发到原本的游标位置设置路径，
// 避免滚轮触发坐标移动。
DWORD fixMouseWheelRetJmpAddr = 0x009E809F;
DWORD fixMouseWheelCallSetCursorPosAddr = 0x0059A0CB;
__declspec(naked) void fixMouseWheelHook() {
    __asm {
        cmp eax, 522
        je next
        mov eax, dword ptr ds : [edi]
        shr eax, 0x10
        push eax
        movzx eax, word ptr ds : [edi]
        push eax
        call fixMouseWheelCallSetCursorPosAddr
        next :
        jmp[fixMouseWheelRetJmpAddr]
    }
}

// ----- 长快捷栏：数据数组 -----
// 默认按键映射（26 个快捷键，每条 4 字节）
unsigned char Array_aDefaultQKM[] = {
    42, 0, 0, 0,
    82, 0, 0, 0,
    71, 0, 0, 0,
    73, 0, 0, 0,
    2, 0, 0, 0,
    3, 0, 0, 0,
    4, 0, 0, 0,
    5, 0, 0, 0,
    6, 0, 0, 0,
    30, 0, 0, 0,
    31, 0, 0, 0,
    32, 0, 0, 0,
    33, 0, 0, 0,
    29, 0, 0, 0,
    83, 0, 0, 0,
    79, 0, 0, 0,
    81, 0, 0, 0,
    16, 0, 0, 0,
    17, 0, 0, 0,
    18, 0, 0, 0,
    19, 0, 0, 0,
    20, 0, 0, 0,
    44, 0, 0, 0,
    45, 0, 0, 0,
    46, 0, 0, 0,
    47, 0, 0, 0,
    52, 0, 0, 0,
};

unsigned char Array_ptShortKeyPos[] = {
    7, 0, 0, 0,
    8, 0, 0, 0,
    42, 0, 0, 0,
    8, 0, 0, 0,
    77, 0, 0, 0,
    8, 0, 0, 0,
    112, 0, 0, 0,
    8, 0, 0, 0,
    147, 0, 0, 0,
    8, 0, 0, 0,
    182, 0, 0, 0,
    8, 0, 0, 0,
    217, 0, 0, 0,
    8, 0, 0, 0,
    252, 0, 0, 0,
    8, 0, 0, 0,
    287, 1, 0, 0,
    8, 0, 0, 0,
    322, 1, 0, 0,
    8, 0, 0, 0,
    357, 1, 0, 0,
    8, 0, 0, 0,
    392, 1, 0, 0,
    8, 0, 0, 0,
    427, 1, 0, 0,
    8, 0, 0, 0,
    7, 0, 0, 0,
    41, 0, 0, 0,
    42, 0, 0, 0,
    41, 0, 0, 0,
    77, 0, 0, 0,
    41, 0, 0, 0,
    112, 0, 0, 0,
    41, 0, 0, 0,
    147, 0, 0, 0,
    41, 0, 0, 0,
    182, 0, 0, 0,
    41, 0, 0, 0,
    217, 0, 0, 0,
    41, 0, 0, 0,
    252, 0, 0, 0,
    41, 0, 0, 0,
    287, 1, 0, 0,
    41, 0, 0, 0,
    322, 1, 0, 0,
    41, 0, 0, 0,
    357, 1, 0, 0,
    41, 0, 0, 0,
    392, 1, 0, 0,
    41, 0, 0, 0,
    427, 1, 0, 0,
    41, 0, 0, 0,
};

unsigned char Array_ptShortKeyPos_Fixed_Tooltips[] = {
    7,0,0,0,0,0,0,0,42,0,0,0,0,0,0,0,77,0,0,0,0,0,0,0,112,0,0,0,0,0,0,0,147,0,0,0,0,0,0,0,182,0,0,0,0,0,0,0,217,0,0,0,0,0,0,0,252,0,0,0,0,0,0,0,287,1,0,0,0,0,0,0,322,1,0,0,0,0,0,0,357,1,0,0,0,0,0,0,392,1,0,0,0,0,0,0,427,1,0,0,0,0,0,0,
    7,0,0,0,33,0,0,0,42,0,0,0,33,0,0,0,77,0,0,0,33,0,0,0,112,0,0,0,33,0,0,0,147,0,0,0,33,0,0,0,182,0,0,0,33,0,0,0,217,0,0,0,33,0,0,0,252,0,0,0,33,0,0,0,287,1,0,0,33,0,0,0,322,1,0,0,33,0,0,0,357,1,0,0,33,0,0,0,392,1,0,0,33,0,0,0,427,1,0,0,33,0,0,0
};

unsigned char Array_aDefaultQKM_0[] = {
    42, 0, 0, 0,
    82, 0, 0, 0,
    71, 0, 0, 0,
    73, 0, 0, 0,
    29, 0, 0, 0,
    83, 0, 0, 0,
    79, 0, 0, 0,
    81, 0, 0, 0,
    42, 0, 0, 0,
    82, 0, 0, 0,
    71, 0, 0, 0,
    73, 0, 0, 0,
    29, 0, 0, 0,
    83, 0, 0, 0,
    79, 0, 0, 0,
    81, 0, 0, 0,
    84, 0, 0, 0,
    85, 0, 0, 0,
    86, 0, 0, 0,
    87, 0, 0, 0,
    88, 0, 0, 0,
    89, 0, 0, 0,
    29, 0, 0, 0,
    29, 0, 0, 0,
    29, 0, 0, 0,
    29, 0, 0, 0,
    29, 0, 0, 0,
};

unsigned char Array_Expanded[312] = { 4, 4, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 1, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 2, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 3, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 5, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 6, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 7, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 8, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 10, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 11, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 12, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 13, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 14, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 15, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 16, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 17, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 23, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 24, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 25, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 26, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    4, 27, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    5, 50, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    5, 51, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    5, 52, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    5, 53, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

unsigned char Array_Expanded_Testing_Cooldown_fix[312] = { 0 };

unsigned char cooldown_Array[124] = { 255, 255, 255, 255, 255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255 };

// ----- 长快捷栏：辅助地址 -----
DWORD Array_aDefaultQKM_Address = (DWORD)&Array_aDefaultQKM;
DWORD Array_mystery_Address = (DWORD)&Array_Expanded;
DWORD Array_mystery_Address_plus = (DWORD)&Array_Expanded + 1;
DWORD cooldown_Array_Address = (DWORD)&cooldown_Array;
DWORD Array_Expanded_Testing_Cooldown_fix_Address = (DWORD)&Array_Expanded_Testing_Cooldown_fix;

// ----- 长快捷栏：代码洞穴 -----
// [推理] 原本 0x8DD8B8 处用 memset 把 quickslot 状态清零为 0x60 字节（容纳 4 个槽），
// 本 cave 把大小改为 0x138（容纳 26 个槽），并跳到原本调用 memset 之后继续执行。
DWORD CompareValidate_Retn = 0x8DD8BD;
_declspec(naked) void CompareValidateFuncKeyMappedInfo_cave() {
    _asm {
        push 0x138
        push 0x0
        push eax
        pushad
        popad
        jmp CompareValidate_Retn
    }
}

// [推理] 该 cave 拦截 CQuickslotKeyMappedMan 的内存分配：
// 第一次进入（eax==0）时分配 0xD4 字节，第二次进入分配 0x138 字节，
// 使得 26 个槽位的按键映射能被装下。
DWORD sub_9FA0CB_cave_retn_1 = 0x9FA0E1;
_declspec(naked) void sub_9FA0CB_cave() {
    _asm {
        test eax, eax
        jne label
        push 0xD4
        pushad
        popad
        jmp sub_9FA0CB_cave_retn_1
    label:
        push 0x138
        push 0x0
        push eax
        pushad
        popad
        jmp CompareValidate_Retn
    }
}

// [推理] 用 Array_aDefaultQKM 替换原本只复制 0x20（4 个槽）的逻辑，
// 改为复制 0x1A * 2 个槽位的默认按键映射到对象内存。
_declspec(naked) void sDefaultQuickslotKeyMap_cave() {
    _asm {
        push ebx
        push esi
        push edi
        xor edx, edx
        mov ebx, ecx
        call label
        nop
        lea edi, dword ptr ds : [ebx + 0x4]
        mov ecx, 0x1A
        mov esi, Array_aDefaultQKM_Address
        rep movsd
        lea edi, dword ptr ds : [ebx + 0x6C]
        mov ecx, 0x1A
        mov esi, Array_aDefaultQKM_Address
        rep movsd
        pop edi
        pop esi
        pop ebx
        ret
    label:
        push esi
        mov esi, ecx
        lea eax, dword ptr ds : [esi + 0x4]
        push 0x72B7C2
        ret
    }
}

// [推理] 把 Array_aDefaultQKM 复制 0x1A 槽位到 CQuickslotKeyMappedMan 对象的成员数组。
_declspec(naked) void DefaultQuickslotKeyMap_cave() {
    _asm {
        push esi
        push edi
        lea eax, dword ptr ds : [ecx + 0x4]
        mov esi, Array_aDefaultQKM_Address
        mov ecx, 0x1A
        mov edi, eax
        rep movsd
        pop edi
        pop esi
        ret
    }
}

// [推理] 把 Array_Expanded_Testing_Cooldown_fix 的内容恢复到 Array_Expanded 中，
// 保持 CD 计时器数据在客户端切换状态时不被清空。
_declspec(naked) void Restore_Array_Expanded() {
    _asm {
        lea eax, [esi + 0D7Ch]
        push esi
        push edi
        push ecx
        mov esi, [Array_Expanded_Testing_Cooldown_fix_Address]
        mov edi, Array_mystery_Address
        mov ecx, 78
        rep movsd
        pop ecx
        pop edi
        pop esi
        push 0x008CFE03
        ret
    }
}

// ----- 日期格式修复（繁体/简体切换相关） -----
// 改写 StringPool 5273 的日期字符串构造方式，避开原生 MM/DD/YYYY 拼接，
// 改为按 YYYY-MM-DD 顺序压栈。
DWORD fixDateFormatRtnAddr = 0x008EBF65;
// [推理] StringPool 5273：在弹窗标题格式化时把 [ebp-1Ch]=年、[ebp-1Ah]=月、
// [ebp-16h]=日（本地变量）按 YYYY-MM-DD 顺序入栈，跳过原 MM/DD/YYYY 拼接。
__declspec(naked) void fixDateFormat() {
    __asm {
        movzx   ecx, word ptr[ebp - 16h]
        push    ecx
        movzx   ecx, word ptr[ebp - 1Ah]
        push    ecx
        movzx   ecx, word ptr[ebp - 1Ch]
        jmp fixDateFormatRtnAddr
    }
}

DWORD fixDateFormat2RtnAddr = 0x008EBFAF;
// [推理] StringPool 655：与 fixDateFormat 相同的入栈顺序，用于第二个日期字符串。
__declspec(naked) void fixDateFormat2() {
    __asm {
        movzx   ecx, word ptr[ebp - 16h]
        push    ecx
        movzx   ecx, word ptr[ebp - 1Ah]
        push    ecx
        movzx   ecx, word ptr[ebp - 1Ch]
        jmp fixDateFormat2RtnAddr
    }
}

DWORD fixDateFormat3RtnAddr = 0x008EC328;
// [推理] StringPool 679：变量栈布局不同，使用 [ebp-24h/-22h/-1Eh] 对应年/月/日。
__declspec(naked) void fixDateFormat3() {
    __asm {
        movzx   ecx, word ptr[ebp - 1Eh]
        push    ecx
        movzx   ecx, word ptr[ebp - 22h]
        push    ecx
        movzx   ecx, word ptr[ebp - 24h]
        jmp fixDateFormat3RtnAddr
    }
}

DWORD fixDateFormat4RtnAddr = 0x008EBF13;
// [推理] StringPool 3138：与 fixDateFormat 相同的变量栈布局，年/月/日依次入栈。
__declspec(naked) void fixDateFormat4() {
    __asm {
        movzx   ecx, word ptr[ebp - 16h]
        push    ecx
        movzx   ecx, word ptr[ebp - 1Ah]
        push    ecx
        movzx   ecx, word ptr[ebp - 1Ch]
        jmp fixDateFormat4RtnAddr
    }
}

// ----- 装备类型识别 -----
// [推理] getItemType1 跳到 getItemType2Addr，避开原 5 位 ID 截断逻辑。
DWORD getItemType2Addr = 0x005CFAC2;
__declspec(naked) void getItemType1() {
    __asm {
        jmp getItemType2Addr
    }
}

// [推理] 5 类装备 type（eqp=0x6D9 / use=0x6E3 / ins=0x0B / etc=0x6DD / cash=0x159C）。
DWORD getItemType2ErrRtnAddr = 0x005CFAA8;
DWORD getItemType2RtnAddr = 0x005CFADD;
__declspec(naked) void getItemType2() {
    __asm {
        dec eax
        jz label_eqp
        dec eax
        jz label_use
        dec eax
        jz label_ins
        dec eax
        jz label_etc
        dec eax
        jz label_cash
        jmp getItemType2ErrRtnAddr
    label_cash:
        push 0x159C
        jmp getItemType2RtnAddr
    label_etc:
        push 0x6DD
        jmp getItemType2RtnAddr
    label_ins:
        push 0x0B
        jmp getItemType2RtnAddr
    label_use:
        push 0x6E3
        jmp getItemType2RtnAddr
    label_eqp:
        push 0x6D9
        jmp getItemType2RtnAddr
    }
}

// ----- 自定义跳跃上限 -----
// [推理] 把 CalcDamageGen 中算出的伤害值（eax）限制在 jumpCap 范围内，
// 避免异常高跳跃触发的连锁伤害溢出。
const DWORD back1 = 0x007807A1;
// [推理] 拦截 0x00780797：若 eax（伤害）> jumpCap，则把 eax 截断为 jumpCap，
// 然后把 edx 设为 edi 继续原流程。
__declspec(naked) void customJumpCapHook1() {
    __asm {
        cmp eax, jumpCap
        jl label
        push jumpCap
        pop eax
    label:
        mov edx, edi
        jmp[back1]
    }
}

const DWORD back2 = 0x008C42AD;
const DWORD back3 = 0x008C42AF;
// [推理] 拦截 0x008C42A3：若 eax> jumpCap 则把 edi 也截断为 jumpCap。
__declspec(naked) void customJumpCapHook2() {
    __asm {
        cmp eax, jumpCap
        jle label
        push jumpCap
        pop edi
        jmp[back3]
    label:
        jmp[back2]
    }
}

const DWORD back4 = 0x0094D947;
// [推理] 拦截 0x0094D942：把 ecx 设为 jumpCap，与 eax 做后续比较，
// 限制跳跃相关的伤害分支。
__declspec(naked) void customJumpCapHook3() {
    __asm {
        push jumpCap
        pop ecx
        cmp eax, ecx
        jmp[back4]
    }
}

// ----- 聊天输入框光标位置 -----
// [推理] 根据聊天输入控件类型（type1/type2/type3）调整光标 X 坐标，
// 让 CJK 字符宽度变化时光标偏移仍正确。
const DWORD chatTextPosRtn = 0x008DD075;
// [推理] 拦截 0x008DD06F：把 [edi+0CFCh] 基准加到 eax 后，
// 按 [edi+0D00h] 类型减 1/0/2 像素修正光标。
__declspec(naked) void chatTextPos() {
    __asm {
        add eax, [edi + 0CFCh]
        cmp[edi + 0D00h], 3
        jz label_type3
        cmp[edi + 0D00h], 2
        jz label_type2
    label_type1:
        sub eax, 1
        jmp label_rtn
    label_type2:
        jmp label_rtn
    label_type3:
        sub eax, 2
    label_rtn:
        jmp chatTextPosRtn
    }
}

// ----- Climb speed hook -----
int curSpeed = 100;
// [推理] 计算当前攀爬速度：把速度限制在 [80, speedMovementCap] 区间，
// 再按 climbSpeed 倍率（最低 1.0）算出 *3 倍的实际攀爬速度。
void calcClimbSpeed() {
    int speed = curSpeed;
    speed = speed < 80 ? 80 : speed;
    speed = speed > speedMovementCap ? speedMovementCap : speed;

    double climbingSpeed = climbSpeed;
    climbingSpeed = climbingSpeed <= 1.0 ? 1.0 : climbingSpeed;
    double curClimbSpeed = 3.0 * speed * climbingSpeed / 100;
    *(double*)0x00C1CF80 = curClimbSpeed;
}

DWORD calcSpeedHookRtn = 0x0094D942;
// [推理] 拦截 0x0094D93C：把 [ebp-10h] 处的速度保存到 curSpeed，
// 调用 calcClimbSpeed 后回填 0x00C1CF80（攀爬速度全局变量），
// 再执行原逻辑的 max(eax, edi) 比较。
__declspec(naked) void calcSpeedHook() {
    __asm {
        push eax
        mov eax, [ebp - 10h]
        mov curSpeed, eax
        call calcClimbSpeed
        pop eax
        cmp     eax, edi
        jg label_return
        mov     eax, edi
    label_return:
        jmp calcSpeedHookRtn
    }
}

// ----- Face / hair model fix -----
// [推理] 原函数 get_equip_data_path 根据传入的 0/2/3/4/5/6 等 type 字段分别走
// 脸型 / 发型加载路径；本 cave 把它们统一重定向到 loc_5C95BF（脸型）、
// loc_5C958D（发型），其余 0x64/0x67 等异常值则保持原处理路径。
DWORD faceRtn = 0x005C95BF;
DWORD hairRtn = 0x005C958D;
DWORD faceHairCaveRtn = 0x005C9505;
__declspec(naked) void faceHairCave() {
    __asm {
        cmp  eax, 2
        jz label_face
        cmp  eax, 3
        jz label_hair
        cmp  eax, 4
        jz label_hair
        cmp  eax, 5
        jz label_face
        cmp  eax, 6
        jz label_hair
        jmp faceHairCaveRtn
    label_face:
        jmp faceRtn
    label_hair:
        jmp hairRtn
    }
}

// ----- canSendPkgTimeCave -----
// [推理] 拦截 CWvsContext::CanSendExclRequest：原逻辑把 (now - [esi+0x20A8])
// 与 arg0 比较，小于则允许发送；本 cave 把它固定为 200ms，作为发包节流阈值。
DWORD canSendPkgTimeCaveRtn = 0x00485C32;
__declspec(naked) void canSendPkgTimeCave() {
    __asm {
        sub eax, [esi + 20A8h]
        cmp eax, 200
        jmp canSendPkgTimeCaveRtn
    }
}

// ----- AP detail button position -----
// [推理] 拦截 CUIStat::OnCreate 中创建 "AP 详情" 按钮的位置参数
// （0x7C=左, 0x144=上），把按钮固定在 (0x144, 0x99) = (324, 153) 的位置。
DWORD apDetailBtnRtn = 0x008C4E22;
__declspec(naked) void apDetailBtn() {
    __asm {
        push    144h
        push    99h
        jmp apDetailBtnRtn
    }
}

// ----- World map -----
// [推理] 拦截 CWorldMapDlg::CreateWorldMapDlg 调用 CDialog::CreateDlg 时的
// 左 / 上坐标（0x29A=左, 0x20C=上），改用 wordMapX / wordMapY 作为锚点。
unsigned char world_cap_increase_array[] = { 0x81, 0xFE };

// wordMapX / wordMapY 定义于 worldmap.cpp（文件作用域，使下方汇编可按名引用）。
DWORD wordMapUIccRtn = 0x009EB5A1;
__declspec(naked) void wordMapUIcc() {
    __asm {
        push 20Ch
        push 29Ah
        push wordMapY
        push wordMapX
        jmp wordMapUIccRtn
    }
}

// ----- Skill tooltip: handle wide CJK chars when wrapping text -----
// [推理] 拦截 sub_8F37C5 中处理技能说明字符串的逻辑：原代码把一个固定宽度
// 写入 [ebp-1Ch] 用于换行宽度计算；本 cave 改为调用 calcCharLen，
// 根据 CJK 双字节字符的实际宽度动态调整换行位置。
int charLen = 55;
void calcCharLen(const char* word) {
    const std::string str = std::string(word);
    if (str.length() < 55) {
        charLen = 55;
        return;
    }
    auto firstByte = static_cast<unsigned char>(str[0]);
    for (int i = 0; i < 60; i++) {
        firstByte = static_cast<unsigned char>(str[i]);
        if (firstByte >= 0x81 && firstByte <= 0xFE) {
            i++; // double-byte CJK char, skip
            continue;
        }
        if (i >= 55) {
            charLen = i;
            break;
        }
    }
}

constexpr DWORD skillToolTipNewRtn = 0x008F3844;
__declspec(naked) void skillToolTipNew() {
    __asm {
        mov eax, [ebp + 0Ch]
        push eax
        call calcCharLen
        pop eax
        mov eax, charLen
        mov[ebp - 1Ch], eax
        lea eax, [ebp - 30h]
        jmp skillToolTipNewRtn
    }
}
