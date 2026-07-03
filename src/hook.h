#pragma once
#include "debug.h"
#include <type_traits>

#ifdef _DEBUG
#define ATTACH_HOOK(TARGET, DETOUR) \
    AttachHook(reinterpret_cast<void**>(&TARGET), CastHook(&DETOUR)) ? true : (ErrorMessage("Failed to attach detour function \"%s\" at target address : 0x%08X.", #DETOUR, TARGET), false)
#else
#define ATTACH_HOOK(TARGET, DETOUR) \
    AttachHook(reinterpret_cast<void**>(&TARGET), CastHook(&DETOUR))
#endif

#define MEMBER_AT(T, OFFSET, NAME) \
    __declspec(property(get = get_##NAME, put = set_##NAME)) T NAME; \
    __forceinline const T& get_##NAME() const { \
        return *reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(this) + OFFSET); \
    } \
    __forceinline T& get_##NAME() { \
        return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OFFSET); \
    } \
    __forceinline void set_##NAME(const T& value) { \
        *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OFFSET) = const_cast<T&>(value); \
    } \
    __forceinline void set_##NAME(T& value) { \
        *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OFFSET) = value; \
    }

#define MEMBER_ARRAY_AT(T, OFFSET, NAME, N) \
    __declspec(property(get = get_##NAME)) T(&NAME)[N]; \
    __forceinline T(&get_##NAME())[N] { \
        return *reinterpret_cast<T(*)[N]>(reinterpret_cast<uintptr_t>(this) + OFFSET); \
    }

#define MEMBER_HOOK(T, ADDRESS, NAME, ...) \
    inline static auto NAME = reinterpret_cast<T(__thiscall*)(void*, __VA_ARGS__)>(ADDRESS); \
    T NAME##_hook(__VA_ARGS__);

#define TO_UINTPTR(VALUE) ((uintptr_t)(VALUE))

#define TO_PVOID(VALUE) ((void*)(VALUE))


// 在 injector.cpp -> DllMain 中调用
void AttachSystemHooks();

// 在 system.cpp -> SetUnhandledExceptionFilter_hook 中调用
void AttachClientBypass();
void AttachClientInlink();
void AttachStringPoolMod();
void AttachResManMod();
void AttachAvatarDataMod();
void AttachItemEffectMod();
void AttachResolutionMod();
void AttachMobHpTagMod();
void AttachToolTipMod();
void AttachIconIconMod();
void AttachTempStatMod();

// 从 BeiDou-ijl15 移植的模块（不含分辨率相关），按功能分组
void AttachClientStartupMod(); // 启动时的内存补丁（IP / 端口 / 上限）
void AttachMouseWheelMod();
void AttachQuickSlotMod();     // 长快捷栏
void AttachDateFormatMod();
void AttachItemTypeMod();
void AttachJumpCapMod();
void AttachChatPosMod();
void AttachPasswordMod();
void AttachWorldMapMod();
void AttachExtrasMod();
void AttachHpMpAlertMod();
void AttachImeFixMod();
void AttachBuddyFixMod();
void AttachNetServiceMod();

inline void AttachClientHooks() {
    AttachClientBypass();
    AttachClientInlink();
    AttachStringPoolMod();
    AttachResManMod();
    AttachAvatarDataMod();
    AttachItemEffectMod();
    AttachResolutionMod();
    AttachMobHpTagMod();
    AttachToolTipMod();
    AttachIconIconMod();
    AttachTempStatMod();
    // 从 BeiDou-ijl15 移植（每个功能文件对应一个入口）
    AttachClientStartupMod();
    AttachMouseWheelMod();
    AttachQuickSlotMod();
    AttachDateFormatMod();
    AttachItemTypeMod();
    AttachJumpCapMod();
    AttachChatPosMod();
    AttachPasswordMod();
    AttachWorldMapMod();
    AttachExtrasMod();
    AttachHpMpAlertMod();
    AttachImeFixMod();
    AttachBuddyFixMod();
    AttachNetServiceMod();
}


template <typename T>
constexpr auto CastHook(T fn) -> void* {
    union {
        T fn;
        void* p;
    } u;
    u.fn = fn;
    return u.p;
}

bool AttachHook(void** ppTarget, void* pDetour);

void* VMTHook(void* pInstance, void* pDetour, size_t uIndex);

void* GetAddress(const char* sModuleName, const char* sProcName);

void* GetAddressByPattern(const char* sModuleName, const char* sPattern);

void PatchMemory(void* pAddress, void* pValue, size_t uSize);

void PatchAllByPattern(void* pStart, void* pEnd, const char* sPattern, void* pValue, size_t uSize);


template <typename T>
void Patch1(T pAddress, unsigned char uValue) {
    PatchMemory(TO_PVOID(pAddress), &uValue, sizeof(uValue));
}

template <typename T>
void Patch4(T pAddress, unsigned int uValue) {
    PatchMemory(TO_PVOID(pAddress), &uValue, sizeof(uValue));
}

template <typename T>
void PatchStr(T pAddress, const char* sValue) {
    PatchMemory(TO_PVOID(pAddress), TO_PVOID(sValue), strlen(sValue));
}

template <typename T, typename U>
void PatchNop(T pAddress, U pDestination) {
    size_t uSize = TO_UINTPTR(pDestination) - TO_UINTPTR(pAddress);
    void* pValue = malloc(uSize);
    memset(pValue, 0x90, uSize);
    PatchMemory(TO_PVOID(pAddress), pValue, uSize);
    free(pValue);
}

template <typename T, typename U>
void PatchJmp(T pAddress, U pDestination) {
    Patch1(pAddress, 0xE9);
    Patch4(pAddress + 1, TO_UINTPTR(pDestination) - TO_UINTPTR(pAddress) - 5);
}

template <typename T, typename U>
void PatchCall(T pAddress, U pDestination, size_t uSize = 5) {
    if (uSize < 5) {
        ErrorMessage("Cannot PatchCall at 0x%08X with uSize = %d", TO_UINTPTR(pAddress), uSize);
        return;
    }
    Patch1(pAddress, 0xE8);
    Patch4(pAddress + 1, TO_UINTPTR(pDestination) - TO_UINTPTR(pAddress) - 5);
    if (uSize > 5) {
        PatchNop(pAddress + 5, pAddress + uSize);
    }
}

template <typename T>
void PatchRetZero(T pAddress) {
    PatchStr(pAddress, "\x33\xC0\xC3");
}

// CodeCave：在 pAddress 处放置一个 jmp 跳转到 pCodeCave，
// 然后把原指令剩余的字节用 nop 填满。
// 参数顺序与 PatchCall / PatchJmp / PatchNop 一致：(目标地址, 跳转目标, 字节数)。
template <typename T, typename U>
void CodeCave(T pAddress, U pCodeCave, size_t uNopSize = 5) {
    if (uNopSize < 5) {
        ErrorMessage("Cannot CodeCave at 0x%08X with uNopSize = %d", TO_UINTPTR(pAddress), uNopSize);
        return;
    }
    PatchJmp(pAddress, pCodeCave);
    if (uNopSize > 5) {
        PatchNop(pAddress + 5, pAddress + uNopSize);
    }
}