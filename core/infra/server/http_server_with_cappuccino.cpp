/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <json.hpp>

#include "../../server/http_server.hpp"
#include "../../vendor/Cappuccino/cappuccino.hpp"
#include "../../util/logger.hpp"
#include "../../service/peer_service.hpp"
#include "../../infra/protobuf/convertor.hpp"

#include "../../consensus/connection/connection.hpp"

#include "../../repository/consensus/transaction_repository.hpp"


namespace http {

    const auto assetName = "PointDemo";

    using nlohmann::json;
    using Request = Cappuccino::Request;
    using Response = Cappuccino::Response;


    using namespace transaction;
    using namespace command;
    using namespace event;
    using namespace object;

    json responseError(std::string message){
        return json({
            {"message", message},
            {"status", 400}
        });
    }

    std::vector<std::string> split(const std::string& str, const std::string& delim) noexcept{
        std::vector<std::string> result;
        std::string::size_type pos = 0;
        while(pos != std::string::npos) {
            auto p = str.find(delim, pos);
            if(p == std::string::npos){
                result.push_back(str.substr(pos));
                break;
            }else{
                result.push_back(str.substr(pos, p - pos));
            }
            pos = p + delim.size();
        }
        return result;
    }

    void server() {
        logger::info("server", "initialize server!");
        Cappuccino::Cappuccino( 0, nullptr);


        Cappuccino::route<Cappuccino::Method::POST>( "/account/register",[](std::shared_ptr<Request> request) -> Response{
            auto res = Response(request);
            auto data = request->json();
            std::string uuid = "";
            if(!data.empty()){
                try{
                    auto publicKey = data["publicKey"].get<std::string>();
                    auto alias     = data["alias"].get<std::string>();
                    auto timestamp = data["timestamp"].get<int>();

                    uuid = hash::sha3_256_hex(publicKey);
                    if(repository::account::findByUuid(uuid).publicKey == "") {

                        auto event = ConsensusEvent<Transaction<Add<object::Account>>>(
                            publicKey.c_str(),
                            publicKey.c_str(),
                            alias.c_str()
                        );

                        event.addTxSignature(
                            peer::getMyPublicKey(),
                            signature::sign(event.getHash(), peer::getMyPublicKey(), peer::getPrivateKey()).c_str()
                        );

                        connection::send(peer::getMyIp(), convertor::encode(event));

                    }else{
                        return responseError("duplicate user");
                    }
                }catch(...) {
                    return responseError("Invalied json type or value");
                }
            }else{
                return responseError("Invalied json");
            }
            res.json(json({
              {"status",  200},
              {"message", "successful"},
              {"uuid",   uuid}
            }));
            return res;
        });



        Cappuccino::route<Cappuccino::Method::GET>( "/account",[](std::shared_ptr<Request> request) -> Response{
            std::string uuid = request->params("uuid");
            auto res = Response(request);

            logger::debug("Cappuccino", "param's uuid is " + uuid);
            object::Account account = repository::account::findByUuid(uuid);
            if(account.publicKey != "") {
                json assets = json::array();
                for (auto &&as: account.assets) {
                    json asset = json::object();
                    asset["value"] = std::get<1>(as);
                    asset["name"] = std::get<0>(as);
                    assets.push_back(asset);
                }

                res.json(json({
                  {"status", 200},
                  {"alias",  account.name},
                  {"assets", assets}
                }));
                return res;
            }else{
                return responseError("User not found!");
            }
        });

        Cappuccino::route<Cappuccino::Method::POST>( "/asset/operation",[](std::shared_ptr<Request> request) -> Response{
            auto res = Response(request);
            auto data = request->json();
            if(!data.empty()){
                try{
                    auto assetUuid = data["asset-uuid"].get<std::string>();
                    auto timestamp = data["timestamp"].get<int>();
                    auto signature = data["signature"].get<std::string>();
                    auto command   = data["params"]["command"].get<std::string>();
                    auto value     = data["params"]["value"].get<std::string>();
                    auto sender    = data["params"]["sender"].get<std::string>();
                    auto receiver  = data["params"]["receiver"].get<std::string>();

                    if(signature::verify(
                        signature,
                        "timestamp:"+std::to_string(timestamp) +\
                        ",params.value:" + value +\
                        ",params.sender:" + sender + \
                        ",params.receiver:" + receiver + \
                        ",params.command:" + command + \
                        ",asset-uuid:" + assetUuid,
                        sender)
                    ){
                        auto event = ConsensusEvent<Transaction<Transfer<object::Asset>>>(
                            sender.c_str(),
                            sender.c_str(),
                            receiver.c_str(),
                            assetName,
                            std::atoi(value.c_str())
                        );
                        event.addTxSignature(
                            peer::getMyPublicKey(),
                            signature::sign(event.getHash(), peer::getMyPublicKey(), peer::getPrivateKey()).c_str()
                        );
                        connection::send(peer::getMyIp(), convertor::encode(event));
                    }else{
                        return responseError("Validation failed!");
                    }
                }catch(...) {
                    return responseError("Invalied json type or value");
                }
            }else{
                return responseError("Invalied json");
            }

            res.json(json({
              {"status",  200},
              {"message", "Ok"}
            }));
            return res;
        });

        Cappuccino::route<Cappuccino::Method::GET>( "/history/transaction",[](std::shared_ptr<Request> request) -> Response{
            std::string uuid = request->params("uuid");
            auto res = Response(request);
            auto tx_json = json::array();

            for(Event::Transaction protoTx: repository::transaction::findAll()){
                json transaction_json = json::object();
                transaction_json["params"] = json::object();

                auto data = split(protoTx.type(),",");

                if(protoTx.type() == "Add"){
                    transaction_json["params"]["command"] = "Add";
                    transaction_json["params"]["sender"] = protoTx.senderpubkey();
                    transaction_json["params"]["timestamp"] = protoTx.timestamp();
                    if(protoTx.has_asset()) {
                        transaction_json["params"]["object"] = "Asset";
                        transaction_json["params"]["value"]  = protoTx.asset().value();
                        transaction_json["params"]["name"]  = protoTx.asset().name();
                    }else if(protoTx.has_account()){
                        transaction_json["params"]["object"] = "Account";
                        transaction_json["params"]["name"]   = protoTx.account().name();
                    }
                    tx_json.push_back(transaction_json);
                }else if(protoTx.type() == "Transfer"){
                    transaction_json["params"]["command"] = "Transfer";
                    transaction_json["params"]["sender"] = protoTx.senderpubkey();
                    transaction_json["params"]["receiver"] = protoTx.receivepubkey();
                    transaction_json["params"]["timestamp"] = protoTx.timestamp();
                    if(protoTx.has_asset()) {
                        auto event_tx = convertor::detail::decodeTransaction2ConsensusEvent<Transfer<Asset>>(protoTx);
                        transaction_json["params"] = json::object();
                        transaction_json["params"]["command"]   = "Transfer";
                        transaction_json["params"]["object"]    = "Asset";
                        transaction_json["params"]["name"]    = protoTx.asset().name();
                        transaction_json["params"]["value"]   = protoTx.asset().value();
                    }
                    tx_json.push_back(transaction_json);
                }
            }

            res.json(json({
              {"status",  200},
              {"history", tx_json}
            }));
            return res;
        });

        logger::info("server", "start server!");
        // runnning
        Cappuccino::run();

    }
};  // namespace http
