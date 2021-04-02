// Copyright (c) 2021 The DeFi Blockchain Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEFI_MASTERNODES_MASTERNODES_COMMON_H
#define DEFI_MASTERNODES_MASTERNODES_COMMON_H

#define PREFIX_CAST(prefix) static_cast<const unsigned char>(prefix)

// @TODO: Change all prefixes to sorted numeric format e.g. 0x01, 0x02, 0x03 etc

enum class DbPrefixes : unsigned char
{
    // Undos DB
    DbPrefixesUndosByUndoKey        = 'u', // 117
    // Gov variables DB
    DbPrefixesGovVarsByName         = 'g', // 103
    // Masternodes DB
    DbPrefixesMasternodeId          = 'M', // 77
    DbPrefixesMnOperator            = 'o', // 111
    DbPrefixesMnOwner               = 'w', // 119
    DbPrefixesMnStaker              = 'X', // 88
    DbPrefixesLastHeight            = 'H', // 72
    DbPrefixesAnchorReward          = 'r', // 114
    DbPrefixesAnchorConfirm         = 'x', // 120
    DbPrefixesCurrentTeam           = 't', // 116
    DbPrefixesFoundersDebt          = 'd', // 100
    DbPrefixesAuthTeam              = 'v', // 118
    DbPrefixesConfirmTeam           = 'V', // 86
    // Tokens DB
    DbPrefixesTokensId              = 'T', // 84
    DbPrefixesTokensSymbol          = 'S', // 83
    DbPrefixesTokensCreationTx      = 'c', // 99
    DbPrefixesTokensLastDctId       = 'L', // 76
    // Accounts DB
    DbPrefixesAccountsByBalanceKey  = 'a', // 97
    // Accounts History DB
    DbPrefixesAccountsHistoryByMine = 'm', // 109
    DbPrefixesAccountsHistoryByAll  = 'h', // 104
    // Rewards History DB
    DbPrefixesRewadsHistoryByMine   = 'Q', // 81
    DbPrefixesRewadsHistoryByAll    = 'W', // 87
    // Poolpairs DB
    DbPrefixesPoolPairById          = 'i', // 105
    DbPrefixesPoolPairByPair        = 'j', // 106
    DbPrefixesPoolPairByShare       = 'k', // 107
    DbPrefixesPoolPairReward        = 'I', // 73
    // Community Balances DB
    DbPrefixesCommunityBalancesById = 'F', // 70
    // Oracles DB
    DbPrefixesOraclesByName         = 'O', // 79
    // Proposals DB
    DbPrefixesCfr                   = 'f', // 102
    DbPrefixesCfrIdsForPaying       = 'p', // 112
};

#endif