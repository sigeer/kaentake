#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// DateFormat - 在 StringPool 条目中把 MM/DD/YYYY 替换为 YYYY-MM-DD。
// 从 BeiDou-ijl15/ezorsia/Client.cpp::FixDateFormat 移植。
// ============================================================================

void DateFormat() {
    CodeCave((void*)0x008EBF57, (void*)fixDateFormat,  14); // StringPool 5273
    CodeCave((void*)0x008EBFA1, (void*)fixDateFormat2, 14); // StringPool 655
    CodeCave((void*)0x008EC31A, (void*)fixDateFormat3, 14); // StringPool 679
    CodeCave((void*)0x008EBF05, (void*)fixDateFormat4, 14); // StringPool 3138
}


void AttachDateFormatMod() {
    DateFormat();
}
