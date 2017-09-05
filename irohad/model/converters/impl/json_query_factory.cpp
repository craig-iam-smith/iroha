/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define RAPIDJSON_HAS_STDSTRING 1

#include "model/converters/json_query_factory.hpp"
#include "common/types.hpp"
#include "model/converters/json_common.hpp"

using namespace rapidjson;

namespace iroha {
  namespace model {
    namespace converters {
      JsonQueryFactory::JsonQueryFactory()
          : log_(logger::log("JsonQueryFactory")) {
        deserializers_["GetAccount"] = &JsonQueryFactory::deserializeGetAccount;
        deserializers_["GetAccountAssets"] =
            &JsonQueryFactory::deserializeGetAccountAssets;
        deserializers_["GetAccountTransactions"] =
            &JsonQueryFactory::deserializeGetAccountTransactions;
        deserializers_["GetAccountAssetTransactions"] =
            &JsonQueryFactory::deserializeGetAccountAssetTransactions;
        deserializers_["GetAccountSignatories"] =
            &JsonQueryFactory::deserializeGetSignatories;
        // Serializers
        serializers_[typeid(GetAccount)] =
            &JsonQueryFactory::serializeGetAccount;
        serializers_[typeid(GetSignatories)] =
            &JsonQueryFactory::serializeGetSignatories;
        serializers_[typeid(GetAccountAssets)] =
            &JsonQueryFactory::serializeGetAccountAssets;
        serializers_[typeid(GetAccountTransactions)] =
            &JsonQueryFactory::serializeGetAccountTransactions;
        serializers_[typeid(GetAccountAssetTransactions)] =
            &JsonQueryFactory::serializeGetAccountAssetTransactions;
      }

      optional_ptr<Query> JsonQueryFactory::deserialize(
          const std::string query_json) {
        return stringToJson(query_json) | [this](auto &json) {
          return this->deserialize(json);
        };
      }

      optional_ptr<Query> JsonQueryFactory::deserialize(
          const rapidjson::Document &document) {
        return deserializeField(document, "query_type", &Value::IsString,
                                &Value::GetString) |
                   [this, &document](auto command_type) -> optional_ptr<Query> {
          auto it = deserializers_.find(command_type);
          if (it != deserializers_.end()) {
            return (this->*deserializers_.at(command_type))(document);
          }
          return nonstd::nullopt;
        } | [&document](auto query) {
          return deserializeField(query, &Query::created_ts,
                                  document, "created_ts", &Value::IsUint64,
                                  &Value::GetUint64);
        } | [&document](auto query) {
          return deserializeField(query, &Query::creator_account_id,
                                  document, "creator_account_id",
                                  &Value::IsString, &Value::GetString);
        } | [&document](auto query) {
          return deserializeField(query, &Query::query_counter,
                                  document, "query_counter", &Value::IsUint64,
                                  &Value::GetUint64);
        } | [&document](auto query) {
          return deserializeField(query, &Query::signature,
                                  document, "signature", &Value::IsObject,
                                  &Value::GetObject);
        } | [this, &document](auto query) {
          query->query_hash = hash_provider_.get_hash(query);
          return nonstd::make_optional(query);
        };
      }

      optional_ptr<Query> JsonQueryFactory::deserializeGetAccount(const
          Value &obj_query) {
        return make_optional_ptr<GetAccount>() | [&obj_query](auto block) {
          return deserializeField(block, &GetAccount::account_id, obj_query,
                                  "account_id", &Value::IsString,
                                  &Value::GetString);
        } | [](auto query) { return optional_ptr<Query>(query); };
      }

      optional_ptr<Query> JsonQueryFactory::deserializeGetSignatories(const
          Value &obj_query) {
        return make_optional_ptr<GetSignatories>() | [&obj_query](auto block) {
          return deserializeField(block, &GetSignatories::account_id, obj_query,
                                  "account_id", &Value::IsString,
                                  &Value::GetString);
        } | [](auto query) { return optional_ptr<Query>(query); };
      }

      optional_ptr<Query> JsonQueryFactory::deserializeGetAccountTransactions(const
          Value &obj_query) {
        return make_optional_ptr<GetAccountTransactions>() | [&obj_query](
                                                                 auto block) {
          return deserializeField(block, &GetAccountTransactions::account_id,
                                  obj_query, "account_id", &Value::IsString,
                                  &Value::GetString);
        } | [](auto query) { return optional_ptr<Query>(query); };
      }

      optional_ptr<Query>
      JsonQueryFactory::deserializeGetAccountAssetTransactions(const
          Value &obj_query) {
        return make_optional_ptr<GetAccountAssetTransactions>() | [&obj_query](
            auto block) {
          return deserializeField(block,
                                  &GetAccountAssetTransactions::account_id,
                                  obj_query, "account_id", &Value::IsString,
                                  &Value::GetString);
        } | [&obj_query](auto block) {
          return deserializeField(block, &GetAccountAssetTransactions::asset_id,
                                  obj_query, "asset_id", &Value::IsString,
                                  &Value::GetString);
        } | [](auto query) { return optional_ptr<Query>(query); };
      }

      optional_ptr<Query> JsonQueryFactory::deserializeGetAccountAssets(const
          Value &obj_query) {
        return make_optional_ptr<GetAccountAssets>() | [&obj_query](
                                                           auto block) {
          return deserializeField(block, &GetAccountAssets::account_id,
                                  obj_query, "account_id", &Value::IsString,
                                  &Value::GetString);
        } | [&obj_query](auto block) {
          return deserializeField(block, &GetAccountAssets::asset_id, obj_query,
                                  "asset_id", &Value::IsString,
                                  &Value::GetString);
        } | [](auto query) { return optional_ptr<Query>(query); };
      }

      // --- Serialization:

      nonstd::optional<std::string> JsonQueryFactory::serialize(
          std::shared_ptr<Query> model_query) {
        Document doc;
        auto &allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("creator_account_id", model_query->creator_account_id,
                      allocator);
        doc.AddMember("query_counter", model_query->query_counter, allocator);
        doc.AddMember("created_ts", model_query->created_ts, allocator);
        Value signature;
        signature.SetObject();
        signature.CopyFrom(serializeSignature(model_query->signature, allocator),
                           allocator);

        doc.AddMember("signature", signature, allocator);

        auto it = serializers_.find(typeid(*model_query));
        if (it != serializers_.end()) {
          (this->*it->second)(doc, model_query);
          return jsonToString(doc);
        }
        log_->error("Query type convertation not implemented");
        return nonstd::nullopt;
      }

      void JsonQueryFactory::serializeGetAccount(Document &json_doc,
                                                 std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccount", allocator);
        auto get_account = std::static_pointer_cast<GetAccount>(query);
        json_doc.AddMember("account_id", get_account->account_id, allocator);
      }
      void JsonQueryFactory::serializeGetAccountAssets(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountAssets", allocator);
        auto casted_query = std::static_pointer_cast<GetAccountAssets>(query);
        json_doc.AddMember("account_id", casted_query->account_id, allocator);
        json_doc.AddMember("asset_id", casted_query->asset_id, allocator);
      }

      void JsonQueryFactory::serializeGetSignatories(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountSignatories", allocator);
        auto get_account = std::static_pointer_cast<GetSignatories>(query);
        json_doc.AddMember("account_id", get_account->account_id, allocator);
      }

      void JsonQueryFactory::serializeGetAccountTransactions(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountTransactions", allocator);
        auto get_account =
            std::static_pointer_cast<GetAccountTransactions>(query);
        json_doc.AddMember("account_id", get_account->account_id, allocator);
      }

      void JsonQueryFactory::serializeGetAccountAssetTransactions(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountAssetTransactions", allocator);
        auto get_account_asset =
            std::static_pointer_cast<GetAccountAssetTransactions>(query);
        json_doc.AddMember("account_id", get_account_asset->account_id, allocator);
        json_doc.AddMember("asset_id", get_account_asset->asset_id, allocator);
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
