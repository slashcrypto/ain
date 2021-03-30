
#include <masternodes/mn_rpc.h>

#include <functional>

/*
 *
 *  Issued by: any
*/
UniValue creategovcfr(const JSONRPCRequest& request)
{
    CWallet* const pwallet = GetWallet(request);

    // TODO implement the RPC according to https://github.com/DeFiCh/pinkpaper/tree/main/governance#rpc
    RPCHelpMan{"creategovcfr",
               "\nCreates a Cummunity Fund Request" +
               HelpRequiringPassphrase(pwallet) + "\n",
               {
                       {"Data", RPCArg::Type::OBJ, RPCArg::Optional::OMITTED_NAMED_ARG,
                        "data in json-form, containing cfr data",
                        {
                            {"title", RPCArg::Type::STR, RPCArg::Optional::NO, "The title of community fund request"},
                            {"finalizeAfter", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "Defaulted to current block height + 70000/2"},
                            {"cycles", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "Defaulted to one cycle"},
                            {"amount", RPCArg::Type::AMOUNT, RPCArg::Optional::NO, "Amount in DFI to request"},
                            {"payoutAddress", RPCArg::Type::STR, RPCArg::Optional::NO, "Any valid address for receiving"},
                        },
                       },
                       {"inputs", RPCArg::Type::ARR, RPCArg::Optional::OMITTED_NAMED_ARG, "A json array of json objects",
                        {
                                {"", RPCArg::Type::OBJ, RPCArg::Optional::OMITTED, "",
                                 {
                                         {"txid", RPCArg::Type::STR_HEX, RPCArg::Optional::NO, "The transaction id"},
                                         {"vout", RPCArg::Type::NUM, RPCArg::Optional::NO, "The output number"},
                                 },
                                },
                        },
                       },
               },
               RPCResult{
                       "\"hash\"                  (string) The hex-encoded hash of broadcasted transaction\n"
               },
               RPCExamples{
                       HelpExampleCli("creategovcfr", "'{\"title\":\"The cfr title\",\"amount\":10,\"payoutAddress\":\"address\"}' '[{\"txid\":\"id\",\"vout\":0}]'")
                       + HelpExampleRpc("creategovcfr", "'{\"title\":\"The cfr title\",\"amount\":10,\"payoutAddress\":\"address\"} '[{\"txid\":\"id\",\"vout\":0}]'")
               },
    }.Check(request);

    if (pwallet->chain().isInitialBlockDownload()) {
        throw JSONRPCError(RPC_CLIENT_IN_INITIAL_DOWNLOAD,
                           "Cannot create a cfr while still in Initial Block Download");
    }
    pwallet->BlockUntilSyncedToCurrentChain();
    LockedCoinsScopedGuard lcGuard(pwallet); // no need here, but for symmetry

    RPCTypeCheck(request.params, { UniValue::VOBJ, UniValue::VARR }, true);

    CAmount amount;
    int cycles = 1;
    std::string title, addressStr;
    auto targetHeight = chainHeight(*pwallet->chain().lock()) + 1;
    auto finalizeAfter = targetHeight + Params().GetConsensus().cfr.votingPeriod / 2;

    const UniValue& data = request.params[0].get_obj();

    if (!data["title"].isNull()) {
        title = data["title"].get_str();
    } else {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "<title> is required");
    }

    if (!data["finalizeAfter"].isNull()) {
        finalizeAfter = data["finalizeAfter"].get_int();
    }

    if (!data["cycles"].isNull()) {
        cycles = data["cycles"].get_int();
    }

    if (!data["amount"].isNull()) {
        amount = data["amount"].get_int();
    } else {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "<amount> is required");
    }

    if (!data["payoutAddress"].isNull()) {
        addressStr = data["payoutAddress"].get_str();
    } else {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "<payoutAddress> is required");
    }

    cycles = std::min(cycles, 10);
    title = title.substr(0, 128);
    finalizeAfter = std::min(finalizeAfter, targetHeight + 3 * Params().GetConsensus().cfr.votingPeriod);

    auto const address = DecodeDestination(addressStr);
    // check type if a supported script
    if (!IsValidDestination(address)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Address (" + addressStr + ") is of an unknown type");
    }

    CCreateCfrMessage cfr;
    cfr.address = GetScriptForDestination(address);
    cfr.nAmount = amount;
    cfr.nCycle = cycles;
    cfr.data = title;

    // encode
    CDataStream metadata(DfTxMarker, SER_NETWORK, PROTOCOL_VERSION);
    metadata << static_cast<unsigned char>(CustomTxType::CreateCfr)
             << cfr;

    CScript scriptMeta;
    scriptMeta << OP_RETURN << ToByteVector(metadata);

    const auto txVersion = GetTransactionVersion(targetHeight);
    CMutableTransaction rawTx(txVersion);

    if (request.params.size() > 2) {
        rawTx.vin = GetInputs(request.params[2].get_array());
    }

    CAmount cfrFee = GetCfrCreationFee(targetHeight, cfr.type);
    rawTx.vout.emplace_back(CTxOut(cfrFee, scriptMeta));

    fund(rawTx, pwallet, {});

    // check execution
    {
        LOCK(cs_main);
        CCustomCSView mnview_dummy(*pcustomcsview); // don't write into actual DB
        const auto res = ApplyCreateCfrTx(mnview_dummy, CTransaction(rawTx), targetHeight,
                                          ToByteVector(CDataStream{SER_NETWORK, PROTOCOL_VERSION, cfr}));
        if (!res.ok) {
            throw JSONRPCError(RPC_INVALID_REQUEST, "Execution test failed:\n" + res.msg);
        }
    }
    return signsend(rawTx, pwallet, {})->GetHash().GetHex();
}

static const CRPCCommand commands[] =
{
//  category        name                     actor (function)        params
//  --------------- ----------------------   ---------------------   ----------
    {"cfr",   "creategovcfr", &creategovcfr, {"address", "amount"} },
};

void RegisterCfrRPCCommands(CRPCTable& tableRPC) {
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
