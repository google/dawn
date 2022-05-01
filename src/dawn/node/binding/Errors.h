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

#ifndef SRC_DAWN_NODE_BINDING_ERRORS_H_
#define SRC_DAWN_NODE_BINDING_ERRORS_H_

#include <string>

#include "src/dawn/node/interop/Napi.h"

namespace wgpu::binding {

// Errors contains static helper methods for creating DOMException error
// messages as documented at:
// https://heycam.github.io/webidl/#idl-DOMException-error-names
class Errors {
  public:
    static Napi::Error HierarchyRequestError(Napi::Env, std::string message = {});
    static Napi::Error WrongDocumentError(Napi::Env, std::string message = {});
    static Napi::Error InvalidCharacterError(Napi::Env, std::string message = {});
    static Napi::Error NoModificationAllowedError(Napi::Env, std::string message = {});
    static Napi::Error NotFoundError(Napi::Env, std::string message = {});
    static Napi::Error NotSupportedError(Napi::Env, std::string message = {});
    static Napi::Error InUseAttributeError(Napi::Env, std::string message = {});
    static Napi::Error InvalidStateError(Napi::Env, std::string message = {});
    static Napi::Error SyntaxError(Napi::Env, std::string message = {});
    static Napi::Error InvalidModificationError(Napi::Env, std::string message = {});
    static Napi::Error NamespaceError(Napi::Env, std::string message = {});
    static Napi::Error SecurityError(Napi::Env, std::string message = {});
    static Napi::Error NetworkError(Napi::Env, std::string message = {});
    static Napi::Error AbortError(Napi::Env, std::string message = {});
    static Napi::Error URLMismatchError(Napi::Env, std::string message = {});
    static Napi::Error QuotaExceededError(Napi::Env, std::string message = {});
    static Napi::Error TimeoutError(Napi::Env, std::string message = {});
    static Napi::Error InvalidNodeTypeError(Napi::Env, std::string message = {});
    static Napi::Error DataCloneError(Napi::Env, std::string message = {});
    static Napi::Error EncodingError(Napi::Env, std::string message = {});
    static Napi::Error NotReadableError(Napi::Env, std::string message = {});
    static Napi::Error UnknownError(Napi::Env, std::string message = {});
    static Napi::Error ConstraintError(Napi::Env, std::string message = {});
    static Napi::Error DataError(Napi::Env, std::string message = {});
    static Napi::Error TransactionInactiveError(Napi::Env, std::string message = {});
    static Napi::Error ReadOnlyError(Napi::Env, std::string message = {});
    static Napi::Error VersionError(Napi::Env, std::string message = {});
    static Napi::Error OperationError(Napi::Env, std::string message = {});
    static Napi::Error NotAllowedError(Napi::Env, std::string message = {});
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_ERRORS_H_
