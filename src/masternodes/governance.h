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

using CCfrId = uint256;

enum class CfrStatus : unsigned char {
  CfrStatusVoting    = 0x01,
  CfrStatusRejected  = 0x02,
  CfrStatusCompleted = 0x03
};

std::string CfrStatusToString(const CfrStatus status);

enum class CfrVoteType : unsigned char {
  CfrVoteTypeYes     = 0x01,
  CfrVoteTypeNo      = 0x02,
  CfrVoteTypeNeutral = 0x03
};

struct CCreateCfrMessage {
    CScript address;
    CAmount amount;
    uint8_t period;
    uint8_t votingSkipPeriod;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(address);
        READWRITE(amount);
        READWRITE(period);
        READWRITE(votingSkipPeriod);
    }
};

struct CVoteCfrMessage {
    CCfrId      cfrId;
    uint8_t     voteType;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(cfrId);
        READWRITE(voteType);
    }
};

struct CCfrVote {
    int64_t     voteTimestamp;
    uint8_t     voteType;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(voteTimestamp);
        READWRITE(voteType);
    }
};

struct CCfrObjectKey
{
    uint8_t   status; // key flag for sorting of processed and unprocessed CFRs objects, this should speed up the search when processing
    uint256   cfrId;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(status);
        READWRITE(cfrId);
    }
};

struct CCfrObject
{
    CAmount amount;
    CScript address;
    uint8_t period;
    uint8_t processedPeriodCount;
    int     finalizeBlockHeight;
    // Voting map: key - masternode ID, value - vote
    std::map<uint256, CCfrVote> votingMap;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(amount);
        READWRITE(address);
        READWRITE(period);
        READWRITE(processedPeriodCount);
        READWRITE(finalizeBlockHeight);
        READWRITE(votingMap);
    }
};

/// View for managing CFRs and their data
class CCfrView : public virtual CStorageView
{
public:
    CCfrView() : _cfrIdsForPayingKey{"cfridsforpaying"} {}

    ~CCfrView() override = default;

    Res CreateCfr(const CCfrId& cfrId, const CScript& address, const CAmount& amount, const uint8_t period);

    ResVal<CCfrObject> GetCfr(const CCfrObjectKey& key) const;

    Res UpdateCfrStatus(const CCfrObjectKey& key, const CfrStatus newStatus);

    Res AddCfrVote(const CCfrId& cfrId, const uint256& masternodeId, const CCfrVote& vote);

    std::set<CCfrId> GetCfrIdsForPaying() const;

    Res AddCfrIdForPaying(const CCfrId& id);

    Res RemoveCfrIdForPaying(const CCfrId& id);

    void ForEachCfr(std::function<bool(CCfrObjectKey const &, CCfrObject const &)> callback, CCfrObjectKey const & start = {}) const;

private:
    struct CfrPrefix {
        static const unsigned char prefix;
    };
    struct IdsForPayingPrefix {
        static const unsigned char prefix;
    };

    const std::string _cfrIdsForPayingKey;
};

#endif