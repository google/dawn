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

#ifndef DAWN_NODE_BINDING_ERRORS_H_
#define DAWN_NODE_BINDING_ERRORS_H_

#include "napi.h"

namespace wgpu { namespace binding {

    // Errors contains static helper methods for creating DOMException error
    // messages as documented at:
    // https://heycam.github.io/webidl/#idl-DOMException-error-names
    class Errors {
      public:
        static Napi::Error HierarchyRequestError(Napi::Env);
        static Napi::Error WrongDocumentError(Napi::Env);
        static Napi::Error InvalidCharacterError(Napi::Env);
        static Napi::Error NoModificationAllowedError(Napi::Env);
        static Napi::Error NotFoundError(Napi::Env);
        static Napi::Error NotSupportedError(Napi::Env);
        static Napi::Error InUseAttributeError(Napi::Env);
        static Napi::Error InvalidStateError(Napi::Env);
        static Napi::Error SyntaxError(Napi::Env);
        static Napi::Error InvalidModificationError(Napi::Env);
        static Napi::Error NamespaceError(Napi::Env);
        static Napi::Error SecurityError(Napi::Env);
        static Napi::Error NetworkError(Napi::Env);
        static Napi::Error AbortError(Napi::Env);
        static Napi::Error URLMismatchError(Napi::Env);
        static Napi::Error QuotaExceededError(Napi::Env);
        static Napi::Error TimeoutError(Napi::Env);
        static Napi::Error InvalidNodeTypeError(Napi::Env);
        static Napi::Error DataCloneError(Napi::Env);
        static Napi::Error EncodingError(Napi::Env);
        static Napi::Error NotReadableError(Napi::Env);
        static Napi::Error UnknownError(Napi::Env);
        static Napi::Error ConstraintError(Napi::Env);
        static Napi::Error DataError(Napi::Env);
        static Napi::Error TransactionInactiveError(Napi::Env);
        static Napi::Error ReadOnlyError(Napi::Env);
        static Napi::Error VersionError(Napi::Env);
        static Napi::Error OperationError(Napi::Env);
        static Napi::Error NotAllowedError(Napi::Env);
    };

}}  // namespace wgpu::binding

#endif  // DAWN_NODE_BINDING_ERRORS_H_
