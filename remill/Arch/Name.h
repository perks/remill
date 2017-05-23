/*
 * Copyright (c) 2017 Trail of Bits, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REMILL_ARCH_NAME_H_
#define REMILL_ARCH_NAME_H_

#include <string>

namespace remill {

enum ArchName : uint32_t {
  kArchInvalid,

  kArchX86,
  kArchX86_AVX,
  kArchX86_AVX512,

  kArchAMD64,
  kArchAMD64_AVX,
  kArchAMD64_AVX512,

  kArchMips32,
  kArchMips64,

  kArchARM,
  kArchARM64
};

// Convert the string name of an architecture into a canonical form.
ArchName GetArchName(const std::string &arch_name);

std::string GetArchName(ArchName);

}  // namespace remill

#endif  // REMILL_ARCH_NAME_H_
