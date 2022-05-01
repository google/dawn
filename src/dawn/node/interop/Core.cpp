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

#include "src/dawn/node/interop/Core.h"

namespace wgpu::interop {

Result Success;

Result Error(std::string msg) {
    return {msg};
}

Result Converter<bool>::FromJS(Napi::Env env, Napi::Value value, bool& out) {
    if (value.IsBoolean()) {
        out = value.ToBoolean();
        return Success;
    }
    return Error("value is not a boolean");
}
Napi::Value Converter<bool>::ToJS(Napi::Env env, bool value) {
    return Napi::Value::From(env, value);
}

Result Converter<std::string>::FromJS(Napi::Env env, Napi::Value value, std::string& out) {
    if (value.IsString()) {
        out = value.ToString();
        return Success;
    }
    return Error("value is not a string");
}
Napi::Value Converter<std::string>::ToJS(Napi::Env env, std::string value) {
    return Napi::Value::From(env, value);
}

Result Converter<int8_t>::FromJS(Napi::Env env, Napi::Value value, int8_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int8_t>::ToJS(Napi::Env env, int8_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint8_t>::FromJS(Napi::Env env, Napi::Value value, uint8_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Uint32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint8_t>::ToJS(Napi::Env env, uint8_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<int16_t>::FromJS(Napi::Env env, Napi::Value value, int16_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int16_t>::ToJS(Napi::Env env, int16_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint16_t>::FromJS(Napi::Env env, Napi::Value value, uint16_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Uint32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint16_t>::ToJS(Napi::Env env, uint16_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<int32_t>::FromJS(Napi::Env env, Napi::Value value, int32_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int32_t>::ToJS(Napi::Env env, int32_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint32_t>::FromJS(Napi::Env env, Napi::Value value, uint32_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Uint32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint32_t>::ToJS(Napi::Env env, uint32_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<int64_t>::FromJS(Napi::Env env, Napi::Value value, int64_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int64Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int64_t>::ToJS(Napi::Env env, int64_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint64_t>::FromJS(Napi::Env env, Napi::Value value, uint64_t& out) {
    if (value.IsNumber()) {
        // Note that the JS Number type only stores doubles, so the max integer
        // range of values without precision loss is -2^53 to 2^53 (52 bit mantissa
        // with 1 implicit bit). This is why there's no UInt64Value() function.
        out = static_cast<uint64_t>(value.ToNumber().Int64Value());
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint64_t>::ToJS(Napi::Env env, uint64_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<float>::FromJS(Napi::Env env, Napi::Value value, float& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().FloatValue();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<float>::ToJS(Napi::Env env, float value) {
    return Napi::Value::From(env, value);
}

Result Converter<double>::FromJS(Napi::Env env, Napi::Value value, double& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().DoubleValue();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<double>::ToJS(Napi::Env env, double value) {
    return Napi::Value::From(env, value);
}

Result Converter<UndefinedType>::FromJS(Napi::Env, Napi::Value value, UndefinedType&) {
    if (value.IsUndefined()) {
        return Success;
    }
    return Error("value is undefined");
}
Napi::Value Converter<UndefinedType>::ToJS(Napi::Env env, UndefinedType) {
    return env.Undefined();
}

}  // namespace wgpu::interop
