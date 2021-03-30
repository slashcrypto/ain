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
enum class CfrVoteType : unsigned char {
  CfrVoteTypeYes     = 0x01,
  CfrVoteTypeNo      = 0x02,
  CfrVoteTypeNeutral = 0x03
};

struct CCreateCfrMessage {
    CAmount amount;
    uint8_t period;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(amount);
        READWRITE(period);
    }
};

struct CVoteCfrMessage {
    CCfrId      cfrId;
    CfrVoteType voteType;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(cfrId);
        READWRITE(voteType);
    }
};

struct CCfrVote {
    int64_t     voteTimestamp;
    CfrVoteType voteType;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(voteTimestamp);
        READWRITE(voteType);
    }
};

struct CCfrObjectKey
{
    CfrStatus status; // key flag for sorting of processed and unprocessed CFRs objects, this should speed up the search when processing
    uint256   cfrId;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(status);
        READWRITE(cfrId);
    }
};

struct CCfrObject
{
    uint8_t period;
    uint8_t processedPeriodCount;
    CAmount amount;
    CScript address;
    std::map<CScript, CCfrVote> votingMap; // CScript meaning the masternode destination address

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(period);
        READWRITE(processedPeriodCount);
        READWRITE(amount);
        READWRITE(address);
        READWRITE(votingMap);
    }
};

/// View for managing CFRs and their data
class CCfrView : public virtual CStorageView
{
public:
    CCfrView();

    ~CCfrView() override = default;

    Res CreateCfr(const CCfrObjectKey& key, const CCfrObject& value);

    Res UpdateCfr(const CCfrObjectKey& key, const CCfrObject& value);

    Res RemoveCfr(const CCfrObjectKey& key);

    ResVal<CCfrObject> GetCfr(const CCfrObjectKey& key) const;

private:
    struct ByName {
        static const unsigned char prefix;
    };
};

#endif