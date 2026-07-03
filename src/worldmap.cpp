#include "pch.h"
#include "hook.h"
#include "codecaves.h"


// ============================================================================
// WorldMap - 提高世界地图绘制列表的上限。从
// BeiDou-ijl15/ezorsia/Client.cpp::WorldMap 移植。
// ============================================================================

// wordMapX/Y 由 codecaves.h 中 wordMapUIcc 的裸汇编按名称引用；
// 将它们保持在文件作用域以使汇编可按名访问。
int wordMapX = 0;
int wordMapY = 0;

void WorldMap() {
    // 世界地图上限提升
    PatchMemory((void*)0x009EA030, world_cap_increase_array, sizeof(world_cap_increase_array));
    Patch4(0x009EA030 + 2, 0xB4);

    wordMapX = 307;   // 中部水平锚点，与游戏分辨率解耦
    wordMapY = 98;    // 中部垂直锚点，与游戏分辨率解耦
    CodeCave((void*)0x009EB594, (void*)wordMapUIcc, 13);
}


void AttachWorldMapMod() {
    WorldMap();
}
