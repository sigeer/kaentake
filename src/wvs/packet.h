#pragma once
#include "hook.h"
#include "wvs/exception.h"
#include "ztl/ztl.h"

#include <winsock2.h>


// ============================================================================
// 封包 / 套接字类型 - 与网络收发相关的所有类集中在 packet.h。
// 不再拆分独立的 wvs/clientsocket.h：ZSocketBase / ZInetAddr 是
// CClientSocket 的字段类型，强行拆到不同头会形成循环 include。
// ============================================================================


// ----- 套接字封装 -----

class ZSocketBase {
private:
    SOCKET _m_hSocket;

public:
    operator SOCKET() {
        return _m_hSocket;
    }
    void CloseSocket() {
        if (_m_hSocket != INVALID_SOCKET) {
            closesocket(_m_hSocket);
            _m_hSocket = INVALID_SOCKET;
        }
    }
    void Socket(int type, int af, int protocol) {
        _m_hSocket = socket(af, type, protocol);
        if (_m_hSocket == INVALID_SOCKET) {
            throw ZException(WSAGetLastError());
        }
    }
};

class ZInetAddr : public sockaddr_in {
public:
    operator const struct sockaddr *() const {
        return (const struct sockaddr*)this;
    }
    operator const struct sockaddr_in *() const {
        return (const struct sockaddr_in*)this;
    }
};

ZRECYCLABLE(ZInetAddr, 0x00BF6A18)


// ----- 封包数据 -----

class CInPacket {
protected:
    int m_bLoopback;
    int m_nState;
    ZArray<unsigned char> m_aRecvBuff;
    unsigned short m_uLength;
    unsigned short m_uRawSeq;
    unsigned short m_uDataLen;
    size_t m_uOffset;
};

static_assert(sizeof(CInPacket) == 0x18);


class COutPacket {
protected:
    int m_bLoopback;
    ZArray<unsigned char> m_aSendBuff;
    unsigned int m_uOffset;
    int m_bIsEncryptedByShanda;

public:
    explicit COutPacket(int nType) : m_aSendBuff(0x100) {
        Init(nType, 0, 0);
    }
    void Encode1(unsigned char n) {
        EncodeBuffer(&n, 1);
    }
    void Encode2(unsigned short n) {
        EncodeBuffer(&n, 2);
    }
    void Encode4(unsigned int n) {
        EncodeBuffer(&n, 4);
    }
    void EncodeStr(ZXString<char> s) {
        int n = s.GetLength();
        Encode2(n);
        EncodeBuffer(s, n);
    }
    void EncodeBuffer(const void* p, size_t uSize) {
        EnlargeBuffer(uSize);
        memcpy(&m_aSendBuff[m_uOffset], p, uSize);
        m_uOffset += uSize;
    }
    void Init(int nType, int bLoopback, int bTypeHeader1Byte) {
        m_bLoopback = bLoopback;
        m_uOffset = 0;
        if (nType != 0x7FFFFFFF) {
            if (bTypeHeader1Byte) {
                Encode1(nType);
            } else {
                Encode2(nType);
            }
        }
        m_bIsEncryptedByShanda = 0;
    }

protected:
    void EnlargeBuffer(size_t uSize) {
        size_t uCur = m_aSendBuff.GetCount();
        size_t uReq = m_uOffset + uSize;
        if (uCur < uReq) {
            do {
                uCur *= 2;
            } while (uCur < uReq);
            m_aSendBuff.Realloc(uCur, 0);
        }
    }
};

static_assert(sizeof(COutPacket) == 0x10);


// ----- 客户端套接字单例 -----

class CClientSocket : public TSingleton<CClientSocket, 0x00BE7914> {
public:
    struct CONNECTCONTEXT {
        ZList<ZInetAddr> lAddr;
        ZInetAddr* posList;
        int bLogin;
    };
    static_assert(sizeof(CONNECTCONTEXT) == 0x1C);

    MEMBER_AT(HWND, 0x4, m_hWnd)
    MEMBER_AT(ZSocketBase, 0x8, m_sock)
    MEMBER_AT(CONNECTCONTEXT, 0xC, m_ctxConnect)
    MEMBER_AT(int, 0x38, m_tTimeout)
    MEMBER_HOOK(void, 0x00494CA3, Connect, const CONNECTCONTEXT& ctx)
    MEMBER_HOOK(void, 0x0049637B, SendPacket, const COutPacket& packet)
    MEMBER_HOOK(void, 0x004965F1, ManipulatePacket, CInPacket* pPacket)
};
