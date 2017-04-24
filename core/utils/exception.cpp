/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdexcept>

#include "exception.hpp"
#include "exception_tag.hpp"

namespace exception {

// Base Exception
IrohaException::IrohaException(const std::string &message)
    : message_(message) {}

IrohaException::~IrohaException() {}

// This is for throwing exception.
const char *IrohaException::what() const throw() {
  return (std::string("<<") + tag() + ">> " + message_).c_str();
}

std::string IrohaException::message() const {
  return (std::string("<<") + tag() + ">> " + message_).c_str();
}

None::None() : IrohaException("This exception should not be used in error") {}

NotImplementedException::NotImplementedException(
    const std::string &functionName, const std::string &filename)
    : IrohaException(
          "Sorry, function [" + functionName + "] in file " + filename +
          " is not yet implemented, would you like to contribute it?") {}

InvalidCastException::InvalidCastException(const std::string &from,
                                           const std::string &to,
                                           const std::string &filename)
    : IrohaException("InvalidCastException in file " + filename +
                     ". Cannot cast from " + from + " to " + to) {}


InvalidCastException::InvalidCastException(const std::string &meg,
                                           const std::string &filename)
    : IrohaException("InvalidCastException in " + filename + ". " + meg) {}

DuplicateSetArgumentException::DuplicateSetArgumentException(
    const std::string &buildTarget, const std::string &duplicateMember)
    : IrohaException("DuplicateSetArgumentException in " + buildTarget +
                     ", argument: " + duplicateMember) {}

UnsetBuildArgumentsException::UnsetBuildArgumentsException(
    const std::string &buildTarget, const std::string &unsetMembers)
    : IrohaException("UnsetBuildArgumentsException in " + buildTarget +
                     ", arguments: " + unsetMembers) {}

NotFoundPathException::NotFoundPathException(const std::string &path)
    : IrohaException("Not found path: '" + path + "'") {}

namespace config {

// deprecated, will remove.
ConfigException::ConfigException(const std::string &message,
                                 const std::string &funcname)
    : IrohaException("ConfigException: " + message + " in " + funcname) {}

ParseException::ParseException(const std::string &target,
                               bool setDefaultMessage)
    : IrohaException("Cannot parse '" + target + "'" +
                     (setDefaultMessage ? " It is set to be default." : "")) {}

UndefinedIrohaHomeException::UndefinedIrohaHomeException()
    : IrohaException(
          "UndefinedIrohaHomeException: Set environment variable IROHA_HOME") {}

}  // namespace config

namespace connection {
NullptrException::NullptrException(const std::string &target)
    : IrohaException("NullptrException: '" + target + "' is nullptr") {}

FailedToCreateConsensusEvent::FailedToCreateConsensusEvent()
    : IrohaException("FailedToCreateConsensusEvent") {}
}  // namespace connection

namespace service {

DuplicationIPException::DuplicationIPException(const std::string &ip)
    : IrohaException("DuplicationIPException : IP = " + ip) {}

DuplicationPublicKeyException::DuplicationPublicKeyException(
    const std::string &publicKey)
    : IrohaException("DuplicationPublicKeyException : publicKey = " +
                     publicKey) {}

UnExistFindPeerException::UnExistFindPeerException(const std::string &publicKey)
    : IrohaException("UnExistFindPeerException : publicKey = " + publicKey) {}

}  // namespace service

namespace crypto {

InvalidKeyException::InvalidKeyException(const std::string &message)
    : IrohaException("Keyfile is invalid, cause is: " + message) {}

InvalidMessageLengthException::InvalidMessageLengthException(
    const std::string &message)
    : IrohaException("Message " + message + " has wrong length") {}

}  // namespace crypto

}  // namespace exception
