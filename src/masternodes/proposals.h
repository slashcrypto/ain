// Copyright (c) DeFi Blockchain Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEFI_MASTERNODES_CFR_H
#define DEFI_MASTERNODES_CFR_H

#include <flushablestorage.h>
#include <masternodes/res.h>
#include <script/script.h>
#include <amount.h>
#include <serialize.h>
#include <uint256.h>
#include <type_traits>

using CCfrId = uint256;

enum class CfrStatus : uint8_t {
  CfrStatusVoting    = 0x01,
  CfrStatusRejected  = 0x02,
  CfrStatusCompleted = 0x03,
};

std::string CfrStatusToString(const CfrStatus status);

enum class CfrVoteType : uint8_t {
  CfrVoteTypeYes     = 0x01,
  CfrVoteTypeNo      = 0x02,
  CfrVoteTypeNeutral = 0x03,
};

struct CCreateCfrMessage {
    CScript address;
    CAmount nAmount;
    uint8_t nCycle;
    std::string data;
    int32_t finalHeight;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(address);
        READWRITE(nAmount);
        READWRITE(nCycle);
        READWRITE(data);
        READWRITE(finalHeight);
    }
};

struct CVoteCfrMessage {
    CCfrId      cfrId;
    CfrVoteType voteType;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(cfrId);
        uint8_t voteTypeByte;
        if (ser_action.ForRead()) {
            READWRITE(voteTypeByte);
            voteType = static_cast<CfrVoteType>(voteTypeByte);
        } else {
            voteTypeByte = static_cast<uint8_t>(voteType);
            READWRITE(voteTypeByte);
        }
    }
};

struct CCfrVote {
    int64_t     voteTimestamp;
    CfrVoteType voteType;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(voteTimestamp);
        uint8_t voteTypeByte;
        if (ser_action.ForRead()) {
            READWRITE(voteTypeByte);
            voteType = static_cast<CfrVoteType>(voteTypeByte);
        } else {
            voteTypeByte = static_cast<uint8_t>(voteType);
            READWRITE(voteTypeByte);
        }
    }
};

struct CCfrObject {
    CAmount amount;
    CScript address;
    uint8_t period;
    uint8_t lastPeriod;
    int32_t finalHeight;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(amount);
        READWRITE(address);
        READWRITE(period);
        READWRITE(lastPeriod);
        READWRITE(finalHeight);
    }
};

/// View for managing proposals and their data
class CCfrView : public virtual CStorageView
{
public:
    ~CCfrView() override = default;

    Res CreateCfr(const CCfrId& cfrId, const CScript& address, CAmount amount, uint8_t period);

    ResVal<CCfrObject> GetCfr(const CCfrId& cfrId) const;

    Res UpdateCfrStatus(const CCfrId& cfrId, CfrStatus newStatus);

    Res AddCfrVote(const CCfrId& cfrId, const uint256& masternodeId, const CCfrVote& vote);

    void ForEachVotingCfr(std::function<bool(CCfrId const &, CCfrObject const &)> callback);
    void ForEachRejectedCfr(std::function<bool(CCfrId const &, CCfrObject const &)> callback);
    void ForEachCompletedCfr(std::function<bool(CCfrId const &, CCfrObject const &)> callback);
    void ForEachFinalizedCfr(std::function<bool(CCfrId const &, CCfrObject const &)> callback, uint32_t height);

    struct ByMnVote { static const unsigned char prefix; };
    struct ByVotingCfr { static const unsigned char prefix; };
    struct ByRejectedCfr { static const unsigned char prefix; };
    struct ByCompletedCfr { static const unsigned char prefix; };
    struct ByFinalizedCfr { static const unsigned char prefix; };
};

#endif
