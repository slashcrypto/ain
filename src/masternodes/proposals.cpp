// Copyright (c) DeFi Blockchain Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <masternodes/proposals.h>
#include <masternodes/masternodes_common.h>

const unsigned char CProposalsView::CfrPrefix::prefix = PREFIX_CAST(DbPrefixes::DbPrefixesCfr);
const unsigned char CProposalsView::CfrIdsForPayingPrefix::prefix = PREFIX_CAST(DbPrefixes::DbPrefixesCfrIdsForPaying);;

std::string CfrStatusToString(const CfrStatus status)
{
    switch(status) {
        case CfrStatus::CfrStatusVoting:
            return "Voting";
        case CfrStatus::CfrStatusRejected:
            return "Rejected";
        case CfrStatus::CfrStatusCompleted:
            return "Completed";
        default:
            return "Undefined";
    }
}

Res CProposalsView::CreateCfr(const CCfrId& cfrId, const CScript& address, const CAmount& amount, const uint8_t period)
{
    CCfrObjectKey key{};
    CCfrObject value{};
    key.cfrId = cfrId;
    key.status = CfrStatus::CfrStatusVoting;
    value.address = address;
    value.amount = amount;
    value.period = period;

    if (!WriteBy<CfrPrefix>(key, value)) {
        return Res::Err("failed to create new CFR <%s>", cfrId.GetHex());
    }
    return Res::Ok();
}

ResVal<CCfrObject> CProposalsView::GetCfr(const CCfrObjectKey& key) const
{
    CCfrObject value{};
    if (!ReadBy<CfrPrefix>(key, value)) {
        return Res::Err("CFR <%s> with status <%s> not found", key.cfrId.GetHex(), CfrStatusToString(key.status));
    }

    return ResVal<CCfrObject>(value, Res::Ok());
}

Res CProposalsView::UpdateCfrStatus(const CCfrObjectKey& key, const CfrStatus newStatus)
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

Res CProposalsView::AddCfrVote(const CCfrId& cfrId, const uint256& masternodeId, const CCfrVote& vote)
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

std::set<CCfrId> CProposalsView::GetCfrIdsForPaying() const
{
    std::set<CCfrId> value;
    if (!ReadBy<CfrIdsForPayingPrefix>(_cfrIdsForPayingKey, value)) {
        return {};
    }

    return value;
}

Res CProposalsView::AddCfrIdForPaying(const CCfrId& id)
{
    std::set<CCfrId> value = GetCfrIdsForPaying();

    value.insert(id);

    if (!WriteBy<CfrIdsForPayingPrefix>(_cfrIdsForPayingKey, value)) {
        return Res::Err("Failed to write CFR ids for paying list");
    }

    return Res::Ok();
}

Res CProposalsView::RemoveCfrIdForPaying(const CCfrId& id)
{
    std::set<CCfrId> value = GetCfrIdsForPaying();

    value.erase(std::find(value.begin(), value.end(), id));

    if (!WriteBy<CfrIdsForPayingPrefix>(_cfrIdsForPayingKey, value)) {
        return Res::Err("Failed to write CFR ids for paying list");
    }

    return Res::Ok();
}