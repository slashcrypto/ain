// Copyright (c) DeFi Blockchain Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <masternodes/proposals.h>

const unsigned char CCfrView::CfrPrefix::prefix = 'f';
const unsigned char CCfrView::CfrIdsForPayingPrefix::prefix = 'p';

std::string CfrStatusToString(const CfrStatus status)
{
    switch(status) {
    case CfrStatus::CfrStatusVoting:    return "Voting";
    case CfrStatus::CfrStatusRejected:  return "Rejected";
    case CfrStatus::CfrStatusCompleted: return "Completed";
    }
    return "Undefined"
}

Res CCfrView::CreateCfr(const CCfrId& cfrId, const CScript& address, const CAmount& amount, const uint8_t period)
{
    CCfrObjectKey key{CfrStatus::CfrStatusVoting, cfrId};
    CCfrObject value{amount, address, period};

    if (!WriteBy<CfrPrefix>(key, value)) {
        return Res::Err("failed to create new CFR <%s>", cfrId.GetHex());
    }
    return Res::Ok();
}

ResVal<CCfrObject> CCfrView::GetCfr(const CCfrObjectKey& key) const
{
    CCfrObject value{};
    if (!ReadBy<CfrPrefix>(key, value)) {
        return Res::Err("CFR <%s> with status <%s> not found", key.cfrId.GetHex(), CfrStatusToString(key.status));
    }

    return ResVal<CCfrObject>(value, Res::Ok());
}

Res CCfrView::UpdateCfrStatus(const CCfrObjectKey& key, const CfrStatus newStatus)
{
    ResVal<CCfrObject> ret = GetCfr(key);

    if (!ret.ok) {
        return Res::Err(ret.msg);
    }

    CCfrObject& value = *ret.val;

    /* remove old entry */
    if (!EraseBy<CfrPrefix>(key)) {
        return Res::Err("Failed to remove CFR <%s> with status <%s>", key.cfrId.GetHex(), CfrStatusToString(key.status));
    }

    CCfrObjectKey newKey{key};
    newKey.status = newStatus;
    /* write entry with new status */

    if(!WriteBy<CfrPrefix>(newKey, value)) {
        return Res::Err("Failed to write CFR <%s> with status <%s>", newKey.cfrId.GetHex(), CfrStatusToString(newKey.status));
    }

    return Res::Ok();
}

Res CCfrView::AddCfrVote(const CCfrId& cfrId, const uint256& masternodeId, const CCfrVote& vote)
{
    const CCfrObjectKey key{CfrStatus::CfrStatusVoting, cfrId};

    ResVal<CCfrObject> ret = GetCfr(key);

    if (!ret.ok) {
        return Res::Err(ret.msg);
    }

    CCfrObject& value = *ret.val;

    value.votingMap.emplace(masternodeId, vote);

    if(!WriteBy<CfrPrefix>(key, value)) {
        return Res::Err("Failed to add vote for CFR <%s> with status <%s>", key.cfrId.GetHex(), CfrStatusToString(key.status));
    }

    return Res::Ok();
}
