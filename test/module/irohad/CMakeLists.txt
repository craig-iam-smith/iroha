# Copyright 2017 Soramitsu Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

# Reusable tests
add_subdirectory(common)
add_subdirectory(ametsuchi)
#add_subdirectory(api)
add_subdirectory(consensus)
add_subdirectory(logger)
add_subdirectory(validation)
add_subdirectory(torii)
add_subdirectory(peerservice)
add_subdirectory(model)
add_subdirectory(synchronizer)
add_subdirectory(simulator)
add_subdirectory(main)
add_subdirectory(ordering)
