// Copyright 2023 The Tint Authors.
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

#include "src/tint/utils/cli/cli.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/text/string.h"

namespace tint::cli {

Option::~Option() = default;

void OptionSet::ShowHelp(std::ostream& s_out) {
    Vector<const Option*, 32> sorted_options;
    for (auto* opt : options.Objects()) {
        sorted_options.Push(opt);
    }
    sorted_options.Sort([](const Option* a, const Option* b) { return a->Name() < b->Name(); });

    struct CmdInfo {
        std::string left;
        std::string right;
    };
    Vector<CmdInfo, 64> cmd_infos;

    for (auto* opt : sorted_options) {
        {
            std::stringstream left, right;
            left << "--" << opt->Name();
            if (auto param = opt->Parameter(); !param.empty()) {
                left << " <" << param << ">";
            }
            right << opt->Description();
            if (auto def = opt->DefaultValue(); !def.empty()) {
                right << "\ndefault: " << def;
            }
            cmd_infos.Push({left.str(), right.str()});
        }
        if (auto alias = opt->Alias(); !alias.empty()) {
            std::stringstream left, right;
            left << "--" << alias;
            right << "alias for --" << opt->Name();
            cmd_infos.Push({left.str(), right.str()});
        }
        if (auto sn = opt->ShortName(); !sn.empty()) {
            std::stringstream left, right;
            left << " -" << sn;
            right << "short name for --" << opt->Name();
            cmd_infos.Push({left.str(), right.str()});
        }
    }

    const size_t kMaxRightOffset = 30;

    // Measure
    size_t left_width = 0;
    for (auto& cmd_info : cmd_infos) {
        for (auto line : tint::Split(cmd_info.left, "\n")) {
            if (line.length() < kMaxRightOffset) {
                left_width = std::max(left_width, line.length());
            }
        }
    }

    // Print
    left_width = std::min(left_width, kMaxRightOffset);

    auto pad = [&](size_t n) {
        while (n--) {
            s_out << " ";
        }
    };

    for (auto& cmd_info : cmd_infos) {
        auto left_lines = tint::Split(cmd_info.left, "\n");
        auto right_lines = tint::Split(cmd_info.right, "\n");

        size_t num_lines = std::max(left_lines.Length(), right_lines.Length());
        for (size_t i = 0; i < num_lines; i++) {
            bool has_left = (i < left_lines.Length()) && !left_lines[i].empty();
            bool has_right = (i < right_lines.Length()) && !right_lines[i].empty();
            if (has_left) {
                s_out << left_lines[i];
                if (has_right) {
                    if (left_lines[i].length() > left_width) {
                        // Left exceeds column width.
                        // Insert a new line and indent to the right
                        s_out << std::endl;
                        pad(left_width);
                    } else {
                        pad(left_width - left_lines[i].length());
                    }
                }
            } else if (has_right) {
                pad(left_width);
            }
            if (has_right) {
                s_out << "  " << right_lines[i];
            }
            s_out << std::endl;
        }
    }
}

Result<OptionSet::Unconsumed> OptionSet::Parse(std::ostream& s_err,
                                               VectorRef<std::string_view> arguments_raw) {
    // Build a map of name to option, and set defaults
    Hashmap<std::string, Option*, 32> options_by_name;
    for (auto* opt : options.Objects()) {
        opt->SetDefault();
        for (auto name : {opt->Name(), opt->Alias(), opt->ShortName()}) {
            if (!name.empty() && !options_by_name.Add(name, opt)) {
                s_err << "multiple options with name '" << name << "'" << std::endl;
                return Failure;
            }
        }
    }

    // Canonicalize arguments by splitting '--foo=x' into '--foo' 'x'.
    std::deque<std::string_view> arguments;
    for (auto arg : arguments_raw) {
        if (HasPrefix(arg, "-")) {
            if (auto eq_idx = arg.find("="); eq_idx != std::string_view::npos) {
                arguments.push_back(arg.substr(0, eq_idx));
                arguments.push_back(arg.substr(eq_idx + 1));
                continue;
            }
        }
        arguments.push_back(arg);
    }

    Hashset<Option*, 8> options_parsed;

    Unconsumed unconsumed;
    while (!arguments.empty()) {
        auto arg = std::move(arguments.front());
        arguments.pop_front();
        auto name = TrimLeft(arg, [](char c) { return c == '-'; });
        if (arg == name || name.length() == 0) {
            unconsumed.Push(arg);
            continue;
        }
        if (auto opt = options_by_name.Find(name)) {
            if (auto err = (*opt)->Parse(arguments); !err.empty()) {
                s_err << err << std::endl;
                return Failure;
            }
        } else {
            s_err << "unknown flag: " << arg << std::endl;
            auto names = options_by_name.Keys();
            auto alternatives =
                Transform(names, [&](const std::string& s) { return std::string_view(s); });
            StringStream ss;
            tint::SuggestAlternativeOptions opts;
            opts.prefix = "--";
            opts.list_possible_values = false;
            SuggestAlternatives(arg, alternatives.Slice(), ss, opts);
            s_err << ss.str();
            return Failure;
        }
    }

    return unconsumed;
}

}  // namespace tint::cli
