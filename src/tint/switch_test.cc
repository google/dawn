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

#include "src/tint/switch.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"

namespace tint {
namespace {

struct Animal : public tint::utils::Castable<Animal> {};
struct Amphibian : public tint::utils::Castable<Amphibian, Animal> {};
struct Mammal : public tint::utils::Castable<Mammal, Animal> {};
struct Reptile : public tint::utils::Castable<Reptile, Animal> {};
struct Frog : public tint::utils::Castable<Frog, Amphibian> {};
struct Bear : public tint::utils::Castable<Bear, Mammal> {};
struct Lizard : public tint::utils::Castable<Lizard, Reptile> {};
struct Gecko : public tint::utils::Castable<Gecko, Lizard> {};
struct Iguana : public tint::utils::Castable<Iguana, Lizard> {};

TEST(Castable, SwitchNoDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool frog_matched_amphibian = false;
        Switch(
            frog.get(),  //
            [&](Reptile*) { FAIL() << "frog is not reptile"; },
            [&](Mammal*) { FAIL() << "frog is not mammal"; },
            [&](Amphibian* amphibian) {
                EXPECT_EQ(amphibian, frog.get());
                frog_matched_amphibian = true;
            });
        EXPECT_TRUE(frog_matched_amphibian);
    }
    {
        bool bear_matched_mammal = false;
        Switch(
            bear.get(),  //
            [&](Reptile*) { FAIL() << "bear is not reptile"; },
            [&](Amphibian*) { FAIL() << "bear is not amphibian"; },
            [&](Mammal* mammal) {
                EXPECT_EQ(mammal, bear.get());
                bear_matched_mammal = true;
            });
        EXPECT_TRUE(bear_matched_mammal);
    }
    {
        bool gecko_matched_reptile = false;
        Switch(
            gecko.get(),  //
            [&](Mammal*) { FAIL() << "gecko is not mammal"; },
            [&](Amphibian*) { FAIL() << "gecko is not amphibian"; },
            [&](Reptile* reptile) {
                EXPECT_EQ(reptile, gecko.get());
                gecko_matched_reptile = true;
            });
        EXPECT_TRUE(gecko_matched_reptile);
    }
}

TEST(Castable, SwitchWithUnusedDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool frog_matched_amphibian = false;
        Switch(
            frog.get(),  //
            [&](Reptile*) { FAIL() << "frog is not reptile"; },
            [&](Mammal*) { FAIL() << "frog is not mammal"; },
            [&](Amphibian* amphibian) {
                EXPECT_EQ(amphibian, frog.get());
                frog_matched_amphibian = true;
            },
            [&](Default) { FAIL() << "default should not have been selected"; });
        EXPECT_TRUE(frog_matched_amphibian);
    }
    {
        bool bear_matched_mammal = false;
        Switch(
            bear.get(),  //
            [&](Reptile*) { FAIL() << "bear is not reptile"; },
            [&](Amphibian*) { FAIL() << "bear is not amphibian"; },
            [&](Mammal* mammal) {
                EXPECT_EQ(mammal, bear.get());
                bear_matched_mammal = true;
            },
            [&](Default) { FAIL() << "default should not have been selected"; });
        EXPECT_TRUE(bear_matched_mammal);
    }
    {
        bool gecko_matched_reptile = false;
        Switch(
            gecko.get(),  //
            [&](Mammal*) { FAIL() << "gecko is not mammal"; },
            [&](Amphibian*) { FAIL() << "gecko is not amphibian"; },
            [&](Reptile* reptile) {
                EXPECT_EQ(reptile, gecko.get());
                gecko_matched_reptile = true;
            },
            [&](Default) { FAIL() << "default should not have been selected"; });
        EXPECT_TRUE(gecko_matched_reptile);
    }
}

TEST(Castable, SwitchDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool frog_matched_default = false;
        Switch(
            frog.get(),  //
            [&](Reptile*) { FAIL() << "frog is not reptile"; },
            [&](Mammal*) { FAIL() << "frog is not mammal"; },
            [&](Default) { frog_matched_default = true; });
        EXPECT_TRUE(frog_matched_default);
    }
    {
        bool bear_matched_default = false;
        Switch(
            bear.get(),  //
            [&](Reptile*) { FAIL() << "bear is not reptile"; },
            [&](Amphibian*) { FAIL() << "bear is not amphibian"; },
            [&](Default) { bear_matched_default = true; });
        EXPECT_TRUE(bear_matched_default);
    }
    {
        bool gecko_matched_default = false;
        Switch(
            gecko.get(),  //
            [&](Mammal*) { FAIL() << "gecko is not mammal"; },
            [&](Amphibian*) { FAIL() << "gecko is not amphibian"; },
            [&](Default) { gecko_matched_default = true; });
        EXPECT_TRUE(gecko_matched_default);
    }
}

TEST(Castable, SwitchMatchFirst) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    {
        bool frog_matched_animal = false;
        Switch(
            frog.get(),
            [&](Animal* animal) {
                EXPECT_EQ(animal, frog.get());
                frog_matched_animal = true;
            },
            [&](Amphibian*) { FAIL() << "animal should have been matched first"; });
        EXPECT_TRUE(frog_matched_animal);
    }
    {
        bool frog_matched_amphibian = false;
        Switch(
            frog.get(),
            [&](Amphibian* amphibain) {
                EXPECT_EQ(amphibain, frog.get());
                frog_matched_amphibian = true;
            },
            [&](Animal*) { FAIL() << "amphibian should have been matched first"; });
        EXPECT_TRUE(frog_matched_amphibian);
    }
}

TEST(Castable, SwitchReturnValueWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        const char* result = Switch(
            frog.get(),                              //
            [](Mammal*) { return "mammal"; },        //
            [](Amphibian*) { return "amphibian"; },  //
            [](Default) { return "unknown"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "amphibian");
    }
    {
        const char* result = Switch(
            bear.get(),                              //
            [](Mammal*) { return "mammal"; },        //
            [](Amphibian*) { return "amphibian"; },  //
            [](Default) { return "unknown"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "mammal");
    }
    {
        const char* result = Switch(
            gecko.get(),                             //
            [](Mammal*) { return "mammal"; },        //
            [](Amphibian*) { return "amphibian"; },  //
            [](Default) { return "unknown"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "unknown");
    }
}

TEST(Castable, SwitchReturnValueWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        const char* result = Switch(
            frog.get(),                        //
            [](Mammal*) { return "mammal"; },  //
            [](Amphibian*) { return "amphibian"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "amphibian");
    }
    {
        const char* result = Switch(
            bear.get(),                        //
            [](Mammal*) { return "mammal"; },  //
            [](Amphibian*) { return "amphibian"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "mammal");
    }
    {
        auto* result = Switch(
            gecko.get(),                       //
            [](Mammal*) { return "mammal"; },  //
            [](Amphibian*) { return "amphibian"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchInferPODReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch(
            frog.get(),                       //
            [](Mammal*) { return 1; },        //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3.0; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 2.0);
    }
    {
        auto result = Switch(
            bear.get(),                       //
            [](Mammal*) { return 1.0; },      //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 1.0);
    }
    {
        auto result = Switch(
            gecko.get(),                   //
            [](Mammal*) { return 1.0f; },  //
            [](Amphibian*) { return 2; },  //
            [](Default) { return 3.0; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 3.0);
    }
}

TEST(Castable, SwitchInferPODReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch(
            frog.get(),                 //
            [](Mammal*) { return 1; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), float>);
        EXPECT_EQ(result, 2.0f);
    }
    {
        auto result = Switch(
            bear.get(),                    //
            [](Mammal*) { return 1.0f; },  //
            [](Amphibian*) { return 2; });
        static_assert(std::is_same_v<decltype(result), float>);
        EXPECT_EQ(result, 1.0f);
    }
    {
        auto result = Switch(
            gecko.get(),                  //
            [](Mammal*) { return 1.0; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 0.0);
    }
}

TEST(Castable, SwitchInferCastableReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch(
            frog.get(),                          //
            [](Mammal* p) { return p; },         //
            [](Amphibian*) { return nullptr; },  //
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Mammal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch(
            bear.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); },
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch(
            gecko.get(),                     //
            [](Mammal* p) { return p; },     //
            [](Amphibian* p) { return p; },  //
            [](Default) -> utils::CastableBase* { return nullptr; });
        static_assert(std::is_same_v<decltype(result), utils::CastableBase*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchInferCastableReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch(
            frog.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian*) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Mammal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch(
            bear.get(),                                                     //
            [](Mammal* p) { return p; },                                    //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); });  //
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch(
            gecko.get(),                  //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return p; });
        static_assert(std::is_same_v<decltype(result), Animal*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchExplicitPODReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch<double>(
            frog.get(),                       //
            [](Mammal*) { return 1; },        //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3.0; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 2.0f);
    }
    {
        auto result = Switch<double>(
            bear.get(),                    //
            [](Mammal*) { return 1; },     //
            [](Amphibian*) { return 2; },  //
            [](Default) { return 3; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 1.0f);
    }
    {
        auto result = Switch<double>(
            gecko.get(),                      //
            [](Mammal*) { return 1.0f; },     //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 3.0f);
    }
}

TEST(Castable, SwitchExplicitPODReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch<double>(
            frog.get(),                 //
            [](Mammal*) { return 1; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 2.0f);
    }
    {
        auto result = Switch<double>(
            bear.get(),                    //
            [](Mammal*) { return 1.0f; },  //
            [](Amphibian*) { return 2; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 1.0f);
    }
    {
        auto result = Switch<double>(
            gecko.get(),                  //
            [](Mammal*) { return 1.0; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 0.0);
    }
}

TEST(Castable, SwitchExplicitCastableReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch<Animal>(
            frog.get(),                          //
            [](Mammal* p) { return p; },         //
            [](Amphibian*) { return nullptr; },  //
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Animal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch<utils::CastableBase>(
            bear.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); },
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), const utils::CastableBase*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch<const Animal>(
            gecko.get(),                     //
            [](Mammal* p) { return p; },     //
            [](Amphibian* p) { return p; },  //
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchExplicitCastableReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch<Animal>(
            frog.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian*) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Animal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch<utils::CastableBase>(
            bear.get(),                                                     //
            [](Mammal* p) { return p; },                                    //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); });  //
        static_assert(std::is_same_v<decltype(result), const utils::CastableBase*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch<const Animal*>(
            gecko.get(),                  //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return p; });
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchNull) {
    Animal* null = nullptr;
    Switch(
        null,  //
        [&](Amphibian*) { FAIL() << "should not be called"; },
        [&](Animal*) { FAIL() << "should not be called"; });
}

TEST(Castable, SwitchNullNoDefault) {
    Animal* null = nullptr;
    bool default_called = false;
    Switch(
        null,  //
        [&](Amphibian*) { FAIL() << "should not be called"; },
        [&](Animal*) { FAIL() << "should not be called"; },
        [&](Default) { default_called = true; });
    EXPECT_TRUE(default_called);
}

TEST(Castable, SwitchReturnNoDefaultInitializer) {
    struct Object {
        explicit Object(int v) : value(v) {}
        int value;
    };

    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    {
        auto result = Switch(
            frog.get(),                            //
            [](Mammal*) { return Object(1); },     //
            [](Amphibian*) { return Object(2); },  //
            [](Default) { return Object(3); });
        static_assert(std::is_same_v<decltype(result), Object>);
        EXPECT_EQ(result.value, 2);
    }
    {
        auto result = Switch(
            frog.get(),                         //
            [](Mammal*) { return Object(1); },  //
            [](Default) { return Object(3); });
        static_assert(std::is_same_v<decltype(result), Object>);
        EXPECT_EQ(result.value, 3);
    }
}

}  // namespace

TINT_INSTANTIATE_TYPEINFO(Animal);
TINT_INSTANTIATE_TYPEINFO(Amphibian);
TINT_INSTANTIATE_TYPEINFO(Mammal);
TINT_INSTANTIATE_TYPEINFO(Reptile);
TINT_INSTANTIATE_TYPEINFO(Frog);
TINT_INSTANTIATE_TYPEINFO(Bear);
TINT_INSTANTIATE_TYPEINFO(Lizard);
TINT_INSTANTIATE_TYPEINFO(Gecko);

}  // namespace tint
