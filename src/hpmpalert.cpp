#include "pch.h"
#include "hook.h"
#include "wvs/config.h"


// ============================================================================
// HpMpAlert - 拦截 CConfig::SaveGlobal，把用户的 HP/MP 警戒阈值
// 发送到服务器。从 BeiDou-ijl15/ezorsia/Client.cpp 移植。
//
// Socket / 单例：复用 wvs/config.h 中的 CConfig 类型（已有
// m_nHpAlert / m_nMpAlert 字段以及 SaveGlobal 的 MEMBER_HOOK），
// 不再维护独立的 UIStatusBarBase / OFS_HPAlert / oSaveGlobal 弱类型封装。
// ============================================================================


// net 模块也会调用的纯函数 helper。
void NetService__PostHpMpAlert(DWORD hp, DWORD mp);


static void SaveGlobal_Hook(void* pThis) {
    CConfig::SaveGlobal(pThis);
    auto pConfig = CConfig::GetInstance();
    if (pConfig) {
        NetService__PostHpMpAlert(pConfig->m_nHpAlert, pConfig->m_nMpAlert);
    }
}


void HpMpAlert() {
    ATTACH_HOOK(CConfig::SaveGlobal, SaveGlobal_Hook);
}

void SetHpMpAlert(unsigned char hpAlert, unsigned char mpAlert) {
    auto pConfig = CConfig::GetInstance();
    if (!pConfig) {
        return;
    }
    pConfig->m_nHpAlert = hpAlert;
    pConfig->m_nMpAlert = mpAlert;
}


void AttachHpMpAlertMod() {
    HpMpAlert();
}
