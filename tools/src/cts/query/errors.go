// Copyright 2022 The Dawn Authors
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

package query

import "fmt"

type ErrNoDataForQuery struct {
	Query Query
}

func (e ErrNoDataForQuery) Error() string {
	return fmt.Sprintf("no data for query '%v'", e.Query)
}

type ErrDuplicateData struct {
	Query Query
}

func (e ErrDuplicateData) Error() string {
	return fmt.Sprintf("duplicate data '%v'", e.Query)
}
