// languagecn.cpp - MapleStory v83 简体中文语言支持模块
//
// === 功能概述 ===============================================================
//   Section 1 - StringPool 字符串翻译
//              替换客户端硬编码字符串（窗口标题、按钮文本、错误信息等）
//   Section 2 - CJK 文本换行处理
//              支持中文文本的正确换行（中文字符无空格分隔，需逐字换行）
//   Section 3 - IME 输入法支持
//              启用中文输入法，允许在聊天栏、搜索框等输入中文
//   Section 4 - IWzFont 工具提示布局 Hook
//              支持 CUIToolTip 工具提示的中文换行（不经过 Section 2）
//
// === 注意事项 ===============================================================
//   1. 字符串编码为 GBK（Chinese Windows 默认代码页 CP_ACP = 936）
//   2. 本文件为单文件模块，所有中文相关代码均在此处
// ============================================================================

#include "pch.h"
#include "stringpool.h"
#include "hook.h"
#include "debug.h"
#include "wvs/wvsapp.h"
#include "ztl/ztl.h"
#include <malloc.h>
#include <imm.h>

#pragma comment(lib, "imm32.lib")

// ============================================================================
// Section 1: StringPool 字符串翻译
// ============================================================================
// 客户端硬编码字符串存储在 XOR 加密的 StringPool 中（字符串表 @ 0x00BDC9D4，
// 16 字节密钥 @ 0x00B001EC）。通过 REPLACE_STRING 宏替换指定索引的字符串。
// ============================================================================
// Section 2: CJK 文本换行处理
// ============================================================================
// 游戏原版的 separate_string_by_width 只按空格（0x20）断行，对没有
// 词间空格的中文文本完全不适用。本 Hook 在遇到 CJK 文本时，在每个
// 中文字符后插入一个空格作为断行标记，让原函数能正确换行，最后再
// 把输出行中多余的尾随空格去除。
//
// CJK 检测方式：直接检查字节是否 >= 0x80（GBK 编码的中文字节特征）。
//
// 注意：此 Hook 覆盖了 separate_string_by_width 的调用方，包括
// string_newline_by_width、CUtilDlg、CEngageDlg 等。但 CUIToolTip
// 类使用的是 IWzFont::CalcLongestTextForGlobal 等字体级布局函数，
// 不经过 separate_string_by_width，因此需要额外的处理方案。

static bool IsCJK(const char* s, int nLen) {
    // 检查字符串中是否存在 GBK 高位字节（>= 0x80），即中文字符
    for (int i = 0; i < nLen; ++i) {
        if (static_cast<unsigned char>(s[i]) >= 0x80) return true;
    }
    return false;
}

// separate_string_by_width 函数原型：
//   void (__cdecl*)(ZArray<ZXString<char>>* pOutLines,
//                   ZXString<char> sText, int nMaxWidth,
//                   IWzFont* pFont);
typedef void (__cdecl* separate_string_by_width_t)(
    void*, void*, int, void*);

static separate_string_by_width_t separate_string_by_width_orig =
    reinterpret_cast<separate_string_by_width_t>(0x00987D56);

static void __cdecl separate_string_by_width_hook(
    void* pOutLines, void* sText, int nMaxWidth, void* pFont)
{
    ZXString<char>& rsText = *static_cast<ZXString<char>*>(sText);
    int nLen = rsText.GetLength();

    // 快速路径：仅含 ASCII 字符的文本使用原始逻辑
    if (nLen == 0 || !IsCJK(rsText, nLen)) {
        separate_string_by_width_orig(pOutLines, sText, nMaxWidth, pFont);
        return;
    }

    // CJK 路径：在每个中文字符后插入空格作为断行机会
    // 分配足够大的缓冲区（原始长度 * 2 + 1 覆盖最坏情况）
    int nNewLen = 0;
    char* sModified = static_cast<char*>(_alloca(nLen * 2 + 1));

    for (int i = 0; i < nLen; ++i) {
        unsigned char c = static_cast<unsigned char>(rsText[static_cast<size_t>(i)]);
        if (c >= 0x81 && c <= 0xFE && i + 1 < nLen) {
            sModified[nNewLen++] = rsText[static_cast<size_t>(i)];
            sModified[nNewLen++] = rsText[static_cast<size_t>(i) + 1];
            sModified[nNewLen++] = 0x20;
            ++i;
        } else {
            sModified[nNewLen++] = rsText[static_cast<size_t>(i)];
        }
    }
    sModified[nNewLen] = 0;

    // 构造新的输入字符串并调用原函数
    ZXString<char> sNewInput(sModified);
    separate_string_by_width_orig(pOutLines,
        reinterpret_cast<void*>(static_cast<char*>(sNewInput)),
        nMaxWidth, pFont);

    // 去除输出行中的尾随空格（我们插入的断行标记）
    ZArray<ZXString<char>>& rOutLines =
        *static_cast<ZArray<ZXString<char>>*>(pOutLines);
    size_t uCount = rOutLines.GetCount();
    for (size_t i = 0; i < uCount; ++i) {
        ZXString<char>& sLine = rOutLines[i];
        int nLineLen = sLine.GetLength();
        while (nLineLen > 0 && sLine[static_cast<size_t>(nLineLen) - 1] == ' ') {
            --nLineLen;
        }
        if (nLineLen < sLine.GetLength()) {
            char* pBuf = sLine.GetBuffer(nLineLen, 1);
            pBuf[nLineLen] = 0;
            sLine.ReleaseBuffer(nLineLen);
        }
    }
}

// ============================================================================
// Section 3: IME 输入法支持
// ============================================================================
// 游戏的 WndProc 不处理 IME 消息，导致无法输入中文。我们 Hook
// CallWindowProcA 函数，将发往游戏主窗口的 IME 消息转发给
// DefWindowProcA，由系统 IMM32/TSF 框架处理。
//
// IME 消息流程：
//   游戏 WndProc_hook → CallWindowProcA（我们的 Hook）→ 原始 WndProc
//   我们在中间截获 IME 消息 → DefWindowProcA → IME 被正确处理

typedef LRESULT (WINAPI* CallWindowProcA_t)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
static CallWindowProcA_t CallWindowProcA_orig = nullptr;

static LRESULT WINAPI CallWindowProcA_hook(
    WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // 仅拦截发往游戏主窗口的 IME 消息
    if (CWvsApp::IsInstantiated() && hWnd == CWvsApp::GetInstance()->m_hWnd) {
        switch (Msg) {

        case WM_IME_STARTCOMPOSITION:
        case WM_IME_ENDCOMPOSITION:
            // 转发给 DefWindowProc 进行默认 IME 处理
            return DefWindowProcA(hWnd, Msg, wParam, lParam);

        case WM_IME_COMPOSITION: {
            // IME 完成组合时，将结果字符串转换为 GBK 并
            // 通过 WM_CHAR 消息发送给游戏
            if (lParam & GCS_RESULTSTR) {
                HIMC hImc = ImmGetContext(hWnd);
                if (hImc) {
                    wchar_t sResult[512] = {};
                    LONG nLen = ImmGetCompositionStringW(hImc,
                        GCS_RESULTSTR, sResult,
                        static_cast<DWORD>(sizeof(sResult) - sizeof(wchar_t)));
                    nLen = nLen / static_cast<LONG>(sizeof(wchar_t));
                    sResult[nLen] = 0;

                    // UTF-16 → 系统 ANSI（GBK）转换后投递 WM_CHAR
                    char sAnsi[1024];
                    int nConv = WideCharToMultiByte(CP_ACP, 0,
                        sResult, nLen, sAnsi,
                        static_cast<int>(sizeof(sAnsi)), nullptr, nullptr);
                    for (int i = 0; i < nConv; ++i) {
                        PostMessageA(hWnd, WM_CHAR,
                            static_cast<WPARAM>(static_cast<unsigned char>(sAnsi[i])), 0);
                    }
                    ImmReleaseContext(hWnd, hImc);
                }
            }
            return DefWindowProcA(hWnd, Msg, wParam, lParam);
        }

        case WM_IME_SETCONTEXT:
            // 隐藏 IME 自带 UI，避免遮挡游戏自己的输入控件
            lParam &= ~ISC_SHOWUIALL;
            return DefWindowProcA(hWnd, Msg, wParam, lParam);

        case WM_IME_NOTIFY:
        case WM_INPUTLANGCHANGE:
            // 让系统处理 IME 通知事件
            return DefWindowProcA(hWnd, Msg, wParam, lParam);
        }
    }
    return CallWindowProcA_orig(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
}

// ============================================================================
// Section 4: IWzFont 工具提示换行 Hook
// ============================================================================
// CUIToolTip 使用 IWzFont 的 CalcLongestTextForGlobal 和
// CalcLineCountForGlobal 进行文本布局。这两个函数不经过
// separate_string_by_width。对于 CJK 文本，通过 Hook 虚表逐字测量
// 宽度来确定正确换行位置。
//
// IWzFont 虚表索引（含 IUnknown 3 个方法）：
//   9  — raw_CalcLongestTextForGlobal
//   11 — raw_CalcLineCountForGlobal

static bool IsCJK_Wide(const wchar_t* ws, int nLen) {
    for (int i = 0; i < nLen; ++i) {
        if (ws[i] >= 0x80) return true;
    }
    return false;
}

typedef HRESULT(__fastcall* CalcLongestTextForGlobal_t)(
    IWzFont*, void*, BSTR, int, VARIANT, int*);
static CalcLongestTextForGlobal_t CalcLongestTextForGlobal_orig = nullptr;

static HRESULT __fastcall CalcLongestTextForGlobal_hook(
    IWzFont* pThis, void* _EDX, BSTR sText, int nWidth,
    VARIANT vTabOrg, int* pnIndex)
{
    int nLen = sText ? SysStringLen(sText) : 0;

    if (nLen == 0 || !IsCJK_Wide(sText, nLen)) {
        return CalcLongestTextForGlobal_orig(
            pThis, _EDX, sText, nWidth, vTabOrg, pnIndex);
    }

    int lo = 0, hi = nLen;
    Ztl_variant_t varTabOrg(vTabOrg);
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        BSTR bstrPrefix = SysAllocStringLen(sText, mid);
        unsigned int uWidth = pThis->CalcTextWidth(bstrPrefix, varTabOrg);
        SysFreeString(bstrPrefix);

        if (uWidth <= static_cast<unsigned int>(nWidth)) {
            lo = mid;
        } else {
            hi = mid - 1;
        }
    }
    if (pnIndex) *pnIndex = lo;
    return S_OK;
}

typedef HRESULT(__fastcall* CalcLineCountForGlobal_t)(
    IWzFont*, void*, BSTR, int, VARIANT, int*);
static CalcLineCountForGlobal_t CalcLineCountForGlobal_orig = nullptr;

static HRESULT __fastcall CalcLineCountForGlobal_hook(
    IWzFont* pThis, void* _EDX, BSTR sText, int nWidth,
    VARIANT vTabOrg, int* pnLineCount)
{
    int nLen = sText ? SysStringLen(sText) : 0;

    if (nLen == 0 || !IsCJK_Wide(sText, nLen)) {
        return CalcLineCountForGlobal_orig(
            pThis, _EDX, sText, nWidth, vTabOrg, pnLineCount);
    }

    int nLines = 0;
    int nPos = 0;
    Ztl_variant_t varTabOrg(vTabOrg);

    while (nPos < nLen) {
        int lo = nPos, hi = nLen;
        while (lo < hi) {
            int mid = (lo + hi + 1) / 2;
            BSTR bstrLine = SysAllocStringLen(sText + nPos, mid - nPos);
            unsigned int uWidth = pThis->CalcTextWidth(bstrLine, varTabOrg);
            SysFreeString(bstrLine);

            if (uWidth <= static_cast<unsigned int>(nWidth)) {
                lo = mid;
            } else {
                hi = mid - 1;
            }
        }
        ++nLines;
        nPos = (lo == nPos) ? nPos + 1 : lo;
    }

    if (pnLineCount) *pnLineCount = nLines;
    return S_OK;
}

static void AttachFontLayoutHooks() {
    IWzFontPtr pFont;
    PcCreateObject<IWzFontPtr>(L"IFont", pFont, nullptr);
    if (!pFont) {
        DEBUG_MESSAGE("  创建 IWzFont 实例失败，跳过字体布局 Hook");
        return;
    }

    void* pOrig = VMTHook(
        static_cast<IWzFont*>(pFont),
        CastHook(&CalcLongestTextForGlobal_hook), 9);
    if (pOrig) {
        CalcLongestTextForGlobal_orig =
            reinterpret_cast<CalcLongestTextForGlobal_t>(pOrig);
        DEBUG_MESSAGE("  IWzFont::raw_CalcLongestTextForGlobal Hook 成功");
    }

    pOrig = VMTHook(
        static_cast<IWzFont*>(pFont),
        CastHook(&CalcLineCountForGlobal_hook), 11);
    if (pOrig) {
        CalcLineCountForGlobal_orig =
            reinterpret_cast<CalcLineCountForGlobal_t>(pOrig);
        DEBUG_MESSAGE("  IWzFont::raw_CalcLineCountForGlobal Hook 成功");
    }

    pFont = nullptr;
}

// ============================================================================
// Section 5：日期格式参数顺序修正
// ============================================================================
// sub_8EBE04 中索引 3138/5273/655 的日期格式字符串使用 M/D/Y 参数顺序
// （wMonth, wDay, wYear）。通过修改 3 个调用点的 movzx 偏移字节，将推栈
// 顺序改为 Y/M/D（wYear, wMonth, wDay），配合替换后的 "%04d年%d月%d日"
// 格式字符串输出正确中文日期。

static void PatchDateFormatArgs() {
    // sub_8EBE04：索引 5273/3138/655（3 个调用点）
    // sub_8EBFDE：索引 679（生命之水）
    //
    // 每个调用点有 3 个连续的 movzx（各 4 字节）+ push（1 字节）：
    //   addr+0:  movzx ecx, word ptr [ebp-XX]  ; 3rd arg（原 wYear）
    //            push ecx
    //   addr+5:  movzx ecx, word ptr [ebp-YY]  ; 2nd arg（原 wDay）
    //            push ecx
    //   addr+10: movzx ecx, word ptr [ebp-ZZ]  ; 1st arg（原 wMonth）
    //            push ecx
    //
    // 修改每个 movzx 指令的第 4 字节（offset）实现 Y/M/D 交换：

    struct PushSite {
        DWORD dwAddr;   // addr+0 = 1st push 序列起始
        BYTE bNewOff0;  // 3rd arg 的新偏移
        BYTE bNewOff1;  // 2nd arg 的新偏移
        BYTE bNewOff2;  // 1st arg 的新偏移
    };

    // addr：      原(ebp)      新(ebp)      循环偏移
    static const PushSite s_Sites[] = {
        // sub_8EBE04：ebp-1Ch(年)→ebp-16h(日), ebp-16h(日)→ebp-1Ah(月), ebp-1Ah(月)→ebp-1Ch(年)
        { 0x008EBF61 - 10, 0xEA, 0xE6, 0xE4 },  // 索引 5273
        { 0x008EBF0F - 10, 0xEA, 0xE6, 0xE4 },  // 索引 3138
        { 0x008EBFAB - 10, 0xEA, 0xE6, 0xE4 },  // 索引 655
        // sub_8EBFDE：ebp-24h(年)→ebp-1Eh(日), ebp-1Eh(日)→ebp-22h(月), ebp-22h(月)→ebp-24h(年)
        { 0x008EC31A,     0xE2, 0xDE, 0xDC },  // 索引 679
    };

    for (const auto& site : s_Sites) {
        DWORD dwOld;
        VirtualProtect(reinterpret_cast<LPVOID>(site.dwAddr), 15,
                       PAGE_EXECUTE_READWRITE, &dwOld);

        *reinterpret_cast<BYTE*>(site.dwAddr + 3)  = site.bNewOff0;  // 3rd arg
        *reinterpret_cast<BYTE*>(site.dwAddr + 8)  = site.bNewOff1;  // 2nd arg
        *reinterpret_cast<BYTE*>(site.dwAddr + 13) = site.bNewOff2;  // 1st arg

        VirtualProtect(reinterpret_cast<LPVOID>(site.dwAddr), 15,
                       dwOld, &dwOld);
        FlushInstructionCache(GetCurrentProcess(),
                              reinterpret_cast<LPVOID>(site.dwAddr), 15);
    }
}

// ============================================================================
// 模块入口
// ============================================================================

void AttachLanguageCNMod() {
    DEBUG_MESSAGE("正在挂载中文语言支持模块...");

    // Section 2：CJK 文本换行 Hook
    ATTACH_HOOK(separate_string_by_width_orig, separate_string_by_width_hook);

    // Section 3：IME 输入法支持（通过 Hook CallWindowProcA）
    CallWindowProcA_orig = reinterpret_cast<CallWindowProcA_t>(
        GetAddress("USER32", "CallWindowProcA"));
    if (CallWindowProcA_orig) {
        ATTACH_HOOK(CallWindowProcA_orig, CallWindowProcA_hook);
    }

    // Section 4：IWzFont 工具提示布局 Hook
    AttachFontLayoutHooks();

    // Section 5：日期格式参数顺序修正（Y/M/D 替换 M/D/Y）
    PatchDateFormatArgs();

    // Section 1：StringPool 字符串翻译
    // 日期格式：月/日/年 → 年/月/日（需配合 PatchDateFormatArgs 交换参数顺序）
    REPLACE_STRING(3138, "可用至 %04d年%d月%d日 %02d:%02d");
    REPLACE_STRING(5273, "封印时间：%04d年%d月%d日 %02d:%02d");
    REPLACE_STRING(655, "可使用至 %04d年%d月%d日 %02d:%02d");
    REPLACE_STRING(679, "生命之水将在 %04d年%d月%d日 %02d:00 失效");


    DEBUG_MESSAGE("中文语言支持模块挂载完成");
}
