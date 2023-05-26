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

#ifndef SRC_TINT_WRITER_SPIRV_IR_TEST_HELPER_IR_H_
#define SRC_TINT_WRITER_SPIRV_IR_TEST_HELPER_IR_H_

#include <string>

#include "gtest/gtest.h"
#include "src/tint/ir/builder.h"
#include "src/tint/writer/spirv/ir/generator_impl_ir.h"
#include "src/tint/writer/spirv/spv_dump.h"

namespace tint::writer::spirv {

/// The element type of a test.
enum TestElementType {
    kBool,
    kI32,
    kU32,
    kF32,
    kF16,
};

/// Base helper class for testing the SPIR-V generator implementation.
template <typename BASE>
class SpvGeneratorTestHelperBase : public BASE {
  public:
    SpvGeneratorTestHelperBase() : generator_(&mod, false) {}

    /// The test module
    ir::Module mod;
    /// The test builder
    ir::Builder b{mod};

  protected:
    /// The SPIR-V generator.
    GeneratorImplIr generator_;

    /// @returns the disassembled types from the generated module.
    std::string DumpTypes() { return DumpInstructions(generator_.Module().Types()); }

    /// Helper to make a scalar type corresponding to the element type `ty`.
    /// @param ty the element type
    /// @returns the scalar type
    const type::Type* MakeScalarType(TestElementType ty) {
        switch (ty) {
            case kBool:
                return mod.Types().bool_();
            case kI32:
                return mod.Types().i32();
            case kU32:
                return mod.Types().u32();
            case kF32:
                return mod.Types().f32();
            case kF16:
                return mod.Types().f16();
        }
        return nullptr;
    }

    /// Helper to make a vector type corresponding to the element type `ty`.
    /// @param ty the element type
    /// @returns the vector type
    const type::Type* MakeVectorType(TestElementType ty) {
        return mod.Types().vec2(MakeScalarType(ty));
    }

    /// Helper to make a scalar value with the scalar type `ty`.
    /// @param ty the element type
    /// @returns the scalar value
    ir::Value* MakeScalarValue(TestElementType ty) {
        switch (ty) {
            case kBool:
                return b.Constant(true);
            case kI32:
                return b.Constant(i32(1));
            case kU32:
                return b.Constant(u32(1));
            case kF32:
                return b.Constant(f32(1));
            case kF16:
                return b.Constant(f16(1));
        }
        return nullptr;
    }

    /// Helper to make a vector value with an element type of `ty`.
    /// @param ty the element type
    /// @returns the vector value
    ir::Value* MakeVectorValue(TestElementType ty) {
        switch (ty) {
            case kBool:
                return b.Constant(mod.constant_values.Composite(
                    MakeVectorType(ty),
                    utils::Vector<const constant::Value*, 2>{mod.constant_values.Get(true),
                                                             mod.constant_values.Get(false)}));
            case kI32:
                return b.Constant(mod.constant_values.Composite(
                    MakeVectorType(ty),
                    utils::Vector<const constant::Value*, 2>{mod.constant_values.Get(i32(42)),
                                                             mod.constant_values.Get(i32(-10))}));
            case kU32:
                return b.Constant(mod.constant_values.Composite(
                    MakeVectorType(ty),
                    utils::Vector<const constant::Value*, 2>{mod.constant_values.Get(u32(42)),
                                                             mod.constant_values.Get(u32(10))}));
            case kF32:
                return b.Constant(mod.constant_values.Composite(
                    MakeVectorType(ty),
                    utils::Vector<const constant::Value*, 2>{mod.constant_values.Get(f32(42)),
                                                             mod.constant_values.Get(f32(-0.5))}));
            case kF16:
                return b.Constant(mod.constant_values.Composite(
                    MakeVectorType(ty),
                    utils::Vector<const constant::Value*, 2>{mod.constant_values.Get(f16(42)),
                                                             mod.constant_values.Get(f16(-0.5))}));
        }
        return nullptr;
    }
};

using SpvGeneratorImplTest = SpvGeneratorTestHelperBase<testing::Test>;

template <typename T>
using SpvGeneratorImplTestWithParam = SpvGeneratorTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_IR_TEST_HELPER_IR_H_
