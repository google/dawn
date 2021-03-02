// Copyright 2020 The Tint Authors.
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

#include "src/castable.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"

namespace tint {
namespace {

struct Animal : public tint::Castable<Animal> {
  explicit Animal(std::string n) : name(n) {}
  const std::string name;
};

struct Amphibian : public tint::Castable<Amphibian, Animal> {
  explicit Amphibian(std::string n) : Base(n) {}
};

struct Mammal : public tint::Castable<Mammal, Animal> {
  explicit Mammal(std::string n) : Base(n) {}
};

struct Reptile : public tint::Castable<Reptile, Animal> {
  explicit Reptile(std::string n) : Base(n) {}
};

struct Frog : public tint::Castable<Frog, Amphibian> {
  Frog() : Base("Frog") {}
};

struct Bear : public tint::Castable<Bear, Mammal> {
  Bear() : Base("Bear") {}
};

struct Gecko : public tint::Castable<Gecko, Reptile> {
  Gecko() : Base("Gecko") {}
};

TEST(CastableBase, Is) {
  std::unique_ptr<CastableBase> frog = std::make_unique<Frog>();
  std::unique_ptr<CastableBase> bear = std::make_unique<Bear>();
  std::unique_ptr<CastableBase> gecko = std::make_unique<Gecko>();

  ASSERT_TRUE(frog->Is<Animal>());
  ASSERT_TRUE(bear->Is<Animal>());
  ASSERT_TRUE(gecko->Is<Animal>());

  ASSERT_TRUE(frog->Is<Amphibian>());
  ASSERT_FALSE(bear->Is<Amphibian>());
  ASSERT_FALSE(gecko->Is<Amphibian>());

  ASSERT_FALSE(frog->Is<Mammal>());
  ASSERT_TRUE(bear->Is<Mammal>());
  ASSERT_FALSE(gecko->Is<Mammal>());

  ASSERT_FALSE(frog->Is<Reptile>());
  ASSERT_FALSE(bear->Is<Reptile>());
  ASSERT_TRUE(gecko->Is<Reptile>());
}

TEST(CastableBase, As) {
  std::unique_ptr<CastableBase> frog = std::make_unique<Frog>();
  std::unique_ptr<CastableBase> bear = std::make_unique<Bear>();
  std::unique_ptr<CastableBase> gecko = std::make_unique<Gecko>();

  ASSERT_EQ(frog->As<Animal>(), static_cast<Animal*>(frog.get()));
  ASSERT_EQ(bear->As<Animal>(), static_cast<Animal*>(bear.get()));
  ASSERT_EQ(gecko->As<Animal>(), static_cast<Animal*>(gecko.get()));

  ASSERT_EQ(frog->As<Amphibian>(), static_cast<Amphibian*>(frog.get()));
  ASSERT_EQ(bear->As<Amphibian>(), nullptr);
  ASSERT_EQ(gecko->As<Amphibian>(), nullptr);

  ASSERT_EQ(frog->As<Mammal>(), nullptr);
  ASSERT_EQ(bear->As<Mammal>(), static_cast<Mammal*>(bear.get()));
  ASSERT_EQ(gecko->As<Mammal>(), nullptr);

  ASSERT_EQ(frog->As<Reptile>(), nullptr);
  ASSERT_EQ(bear->As<Reptile>(), nullptr);
  ASSERT_EQ(gecko->As<Reptile>(), static_cast<Reptile*>(gecko.get()));
}

TEST(Castable, Is) {
  std::unique_ptr<Animal> frog = std::make_unique<Frog>();
  std::unique_ptr<Animal> bear = std::make_unique<Bear>();
  std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();

  ASSERT_TRUE(frog->Is<Animal>());
  ASSERT_TRUE(bear->Is<Animal>());
  ASSERT_TRUE(gecko->Is<Animal>());

  ASSERT_TRUE(frog->Is<Amphibian>());
  ASSERT_FALSE(bear->Is<Amphibian>());
  ASSERT_FALSE(gecko->Is<Amphibian>());

  ASSERT_FALSE(frog->Is<Mammal>());
  ASSERT_TRUE(bear->Is<Mammal>());
  ASSERT_FALSE(gecko->Is<Mammal>());

  ASSERT_FALSE(frog->Is<Reptile>());
  ASSERT_FALSE(bear->Is<Reptile>());
  ASSERT_TRUE(gecko->Is<Reptile>());
}

TEST(Castable, As) {
  std::unique_ptr<Animal> frog = std::make_unique<Frog>();
  std::unique_ptr<Animal> bear = std::make_unique<Bear>();
  std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();

  ASSERT_EQ(frog->As<Animal>(), static_cast<Animal*>(frog.get()));
  ASSERT_EQ(bear->As<Animal>(), static_cast<Animal*>(bear.get()));
  ASSERT_EQ(gecko->As<Animal>(), static_cast<Animal*>(gecko.get()));

  ASSERT_EQ(frog->As<Amphibian>(), static_cast<Amphibian*>(frog.get()));
  ASSERT_EQ(bear->As<Amphibian>(), nullptr);
  ASSERT_EQ(gecko->As<Amphibian>(), nullptr);

  ASSERT_EQ(frog->As<Mammal>(), nullptr);
  ASSERT_EQ(bear->As<Mammal>(), static_cast<Mammal*>(bear.get()));
  ASSERT_EQ(gecko->As<Mammal>(), nullptr);

  ASSERT_EQ(frog->As<Reptile>(), nullptr);
  ASSERT_EQ(bear->As<Reptile>(), nullptr);
  ASSERT_EQ(gecko->As<Reptile>(), static_cast<Reptile*>(gecko.get()));
}

}  // namespace

TINT_INSTANTIATE_TYPEINFO(Animal);
TINT_INSTANTIATE_TYPEINFO(Amphibian);
TINT_INSTANTIATE_TYPEINFO(Mammal);
TINT_INSTANTIATE_TYPEINFO(Reptile);
TINT_INSTANTIATE_TYPEINFO(Frog);
TINT_INSTANTIATE_TYPEINFO(Bear);
TINT_INSTANTIATE_TYPEINFO(Gecko);

}  // namespace tint
