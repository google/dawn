// Copyright 2023 The Dawn Authors
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

// Used to define the DLL version metadata for our built version of dxcompiler.dll,
// overriding certain fields to clarify that this version is built for Dawn.

// Include official DXC version.inc
#include "..\..\dxc\utils\version\version.inc"

// Override fields
#undef RC_FILE_DESCRIPTION
#define RC_FILE_DESCRIPTION "DirectX Compiler - Google Dawn Custom Build"

#undef RC_PRODUCT_NAME
#define RC_PRODUCT_NAME "Microsoft(r) DirectX for Windows(r) - Google Dawn Custom Build"
