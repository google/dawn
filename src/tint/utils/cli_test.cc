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

#include "src/tint/utils/cli.h"

#include <sstream>

#include "gmock/gmock.h"
#include "src/tint/utils/string.h"

#include "src/tint/utils/transform.h"  // Used by ToStringList()

namespace tint::utils::cli {
namespace {

// Workaround for https://github.com/google/googletest/issues/3081
// Remove when using C++20
template <size_t N>
utils::Vector<std::string, N> ToStringList(const utils::Vector<std::string_view, N>& views) {
    return Transform(views, [](std::string_view view) { return std::string(view); });
}

using CLITest = testing::Test;

TEST_F(CLITest, ShowHelp_ValueWithParameter) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value");

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
)");
}

TEST_F(CLITest, ShowHelp_ValueWithAlias) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value", Alias{"alias"});

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
--alias              alias for --my_option
)");
}
TEST_F(CLITest, ShowHelp_ValueWithShortName) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value", ShortName{"a"});

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
 -a                  short name for --my_option
)");
}

TEST_F(CLITest, ShowHelp_MultilineDesc) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("an-option", R"(this is a
multi-line description
for an option
)");

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--an-option <value>  this is a
                     multi-line description
                     for an option

)");
}

TEST_F(CLITest, ShowHelp_LongName) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("an-option-with-a-really-really-long-name",
                               "this is an option that has a silly long name", ShortName{"a"});

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--an-option-with-a-really-really-long-name <value>
     this is an option that has a silly long name
 -a  short name for --an-option-with-a-really-really-long-name
)");
}

TEST_F(CLITest, ShowHelp_EnumValue) {
    enum class E { X, Y, Z };

    OptionSet opts;
    opts.Add<EnumOption<E>>("my_enum_option", "sets the awesome value",
                            utils::Vector{
                                EnumName(E::X, "X"),
                                EnumName(E::Y, "Y"),
                                EnumName(E::Z, "Z"),
                            });

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_enum_option <X|Y|Z>  sets the awesome value
)");
}

TEST_F(CLITest, ShowHelp_MixedValues) {
    enum class E { X, Y, Z };

    OptionSet opts;

    opts.Add<ValueOption<int>>("option-a", "an integer");
    opts.Add<BoolOption>("option-b", "a boolean");
    opts.Add<EnumOption<E>>("option-c", "sets the awesome value",
                            utils::Vector{
                                EnumName(E::X, "X"),
                                EnumName(E::Y, "Y"),
                                EnumName(E::Z, "Z"),
                            });

    std::stringstream out;
    out << std::endl;
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--option-a <value>  an integer
--option-b <value>  a boolean
--option-c <X|Y|Z>  sets the awesome value
)");
}

TEST_F(CLITest, ParseBool_Flag) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option unconsumed", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre("unconsumed"));
    EXPECT_EQ(opt.value, true);
}

TEST_F(CLITest, ParseBool_ExplicitTrue) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option true", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, true);
}

TEST_F(CLITest, ParseBool_ExplicitFalse) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value", Default{true});

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option false", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, false);
}

TEST_F(CLITest, ParseInt) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int>>("my_option", "an integer value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option 42", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 42);
}

TEST_F(CLITest, ParseUint64) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<uint64_t>>("my_option", "a uint64_t value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option 1000000", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 1000000);
}

TEST_F(CLITest, ParseFloat) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<float>>("my_option", "a float value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option 1.25", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 1.25f);
}

TEST_F(CLITest, ParseString) {
    OptionSet opts;
    auto& opt = opts.Add<StringOption>("my_option", "a string value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option blah", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, "blah");
}

TEST_F(CLITest, ParseEnum) {
    enum class E { X, Y, Z };

    OptionSet opts;
    auto& opt = opts.Add<EnumOption<E>>("my_option", "sets the awesome value",
                                        utils::Vector{
                                            EnumName(E::X, "X"),
                                            EnumName(E::Y, "Y"),
                                            EnumName(E::Z, "Z"),
                                        });
    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option Y", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, E::Y);
}

TEST_F(CLITest, ParseShortName) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int>>("my_option", "an integer value", ShortName{"o"});

    std::stringstream err;
    auto res = opts.Parse(err, Split("-o 42", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 42);
}

TEST_F(CLITest, ParseUnconsumed) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int32_t>>("my_option", "a int32_t value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("abc --my_option -123 def", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre("abc", "def"));
    EXPECT_EQ(opt.value, -123);
}

TEST_F(CLITest, ParseUsingEquals) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int>>("my_option", "an int value");

    std::stringstream err;
    auto res = opts.Parse(err, Split("--my_option=123", " "));
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 123);
}

TEST_F(CLITest, SetValueToDefault) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value", Default{true});

    std::stringstream err;
    auto res = opts.Parse(err, utils::Empty);
    ASSERT_TRUE(res) << err.str();
    EXPECT_TRUE(err.str().empty());
    EXPECT_EQ(opt.value, true);
}

}  // namespace
}  // namespace tint::utils::cli
