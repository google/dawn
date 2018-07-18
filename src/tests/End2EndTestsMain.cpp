// Copyright 2017 The Dawn Authors
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

#include <gtest/gtest.h>

extern bool gTestUsesWire;

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    for (int i = 1; i < argc; ++i) {
        if (strcmp("-w", argv[i]) == 0 || strcmp("--use-wire", argv[i]) == 0) {
            gTestUsesWire = true;
            continue;
        }

        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            printf("\n\nUsage: %s [GTEST_FLAGS...] [-w] \n", argv[0]);
            printf("  -w, --use-wire: Run the tests through the wire (defaults to no wire)\n");
            return 0;
        }
    }

    return RUN_ALL_TESTS();
}
