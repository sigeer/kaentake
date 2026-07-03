#include "pch.h"
#include "hook.h"
#include "wvs/packet.h"


// ============================================================================
// NetService - 客户端主动发送的封包。从 BeiDou-ijl15/ezorsia/Net.h 移植。
//
// Socket / 单例：复用 wvs/clientsocket.h 中的 CClientSocket 类型
// （已扩展 SendPacket / ManipulatePacket 的 MEMBER_HOOK），避免在 net.cpp
// 里维护独立的 g_pClientSocket / PacketSend_t / PacketRecv_t 弱类型封装。
// ============================================================================


class NetService {
public:
    // 向服务器发送语言偏好封包（操作码 0x1000）。
    static void PostLanguage(unsigned char language);

    // 向服务器发送可配置的 HP/MP 警戒阈值封包（操作码 0x1001）。
    static void PostHpMpAlert(DWORD hpRatio, DWORD mpRatio);

    // 拦截 CClientSocket::ManipulatePacket 的入口；当前仅做空转，
    // 因为本项目 CInPacket 没有公开的 OpCode getter（BeiDou 偏移
    // 假设不适用于本项目布局），后续接 protocol 解析时再补充。
    static void ConfigureRecvPacket();
};


// hpmpalert.cpp 的 CConfig::SaveGlobal hook 调用的纯函数 helper。
void NetService__PostHpMpAlert(DWORD hp, DWORD mp);


static void RecvHookInternal(void* pThis, CInPacket* pPacket) {
    // [推理] BeiDou-ijl15 在此分支里读取 op code 并发送语言偏好、
    // 写入 HP/MP 警戒；本项目 CInPacket 内存布局与之不同，暂无
    // 公开 API。保留 hook 入口以便后续接入 protocol 解析。
    (void)pThis;
    (void)pPacket;
    CClientSocket::ManipulatePacket(pThis, pPacket);
}


void NetService::PostLanguage(unsigned char language) {
    COutPacket writer(0x1000);
    writer.Encode1(language);
    if (auto* pClientSocket = CClientSocket::GetInstance()) {
        CClientSocket::SendPacket(pClientSocket, writer);
    }
}

void NetService::PostHpMpAlert(DWORD hpRatio, DWORD mpRatio) {
    COutPacket writer(0x1001);
    writer.Encode1(static_cast<unsigned char>(hpRatio));
    writer.Encode1(static_cast<unsigned char>(mpRatio));
    if (auto* pClientSocket = CClientSocket::GetInstance()) {
        CClientSocket::SendPacket(pClientSocket, writer);
    }
}

void NetService__PostHpMpAlert(DWORD hp, DWORD mp) {
    NetService::PostHpMpAlert(hp, mp);
}

void NetService::ConfigureRecvPacket() {
    ATTACH_HOOK(CClientSocket::ManipulatePacket, RecvHookInternal);
}


void AttachNetServiceMod() {
    NetService::ConfigureRecvPacket();
}
