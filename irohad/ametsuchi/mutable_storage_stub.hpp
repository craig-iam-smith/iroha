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

#ifndef IROHA_MUTABLE_STORAGE_STUB_HPP
#define IROHA_MUTABLE_STORAGE_STUB_HPP

#include <ametsuchi/mutable_storage.hpp>
#include <ametsuchi/ametsuchi_stub.hpp>
#include <ametsuchi/command_executor_stub.hpp>

namespace iroha {
  namespace ametsuchi {
    class MutableStorageStub : public MutableStorage {
     public:
      MutableStorageStub(AmetsuchiStub& ametsuchi);
      bool apply(dao::Block block,
                 std::function<bool(dao::Block&, CommandExecutor&, WsvQuery&)>
                 function) override;
      rxcpp::observable<dao::Transaction> get_account_transactions(
          ed25519::pubkey_t pub_key) override;
      rxcpp::observable<dao::Transaction> get_asset_transactions(
          std::string asset_full_name) override;
      rxcpp::observable<dao::Transaction> get_wallet_transactions(
          std::string wallet_id) override;

      dao::Peer get_peer(ed25519::pubkey_t pub_key) override;
      dao::Account get_account(
          ed25519::pubkey_t pub_key) override;
      dao::Asset get_asset(std::string asset_full_name) override;
      dao::Domain get_domain(std::string domain_full_name) override;
      dao::Wallet get_wallet(std::string wallet_id) override;
      std::vector<dao::Wallet> get_account_wallets(
          ed25519::pubkey_t pub_key) override;
      std::vector<dao::Asset> get_domain_assets(
          std::string domain_full_name) override;
     private:
      AmetsuchiStub& ametsuchi_;
      CommandExecutorStub executor_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_MUTABLE_STORAGE_STUB_HPP
