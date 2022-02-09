// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESTS_TOGGLEPARSER_H_
#define TESTS_TOGGLEPARSER_H_

#include <string>
#include <vector>

class ToggleParser {
  public:
    ToggleParser();
    ~ToggleParser();

    bool ParseEnabledToggles(char* arg);
    bool ParseDisabledToggles(char* arg);

    const std::vector<std::string>& GetEnabledToggles() const;
    const std::vector<std::string>& GetDisabledToggles() const;

  private:
    std::vector<std::string> mEnabledToggles;
    std::vector<std::string> mDisabledToggles;
};

#endif  // TESTS_TOGGLEPARSER_H_
