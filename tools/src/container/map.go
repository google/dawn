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

package container

import "sort"

// Map is a generic unordered map, which wrap's go's builtin 'map'.
// K is the map key, which must match the 'key' constraint.
// V is the map value, which can be any type.
type Map[K key, V any] map[K]V

// Returns a new empty map
func NewMap[K key, V any]() Map[K, V] {
	return make(Map[K, V])
}

// Add adds an item to the map.
func (m Map[K, V]) Add(k K, v V) {
	m[k] = v
}

// Remove removes an item from the map
func (m Map[K, V]) Remove(item K) {
	delete(m, item)
}

// Contains returns true if the map contains the given item
func (m Map[K, V]) Contains(item K) bool {
	_, found := m[item]
	return found
}

// Keys returns the sorted keys of the map as a slice
func (m Map[K, V]) Keys() []K {
	out := make([]K, 0, len(m))
	for v := range m {
		out = append(out, v)
	}
	sort.Slice(out, func(i, j int) bool { return out[i] < out[j] })
	return out
}

// Values returns the values of the map sorted by key
func (m Map[K, V]) Values() []V {
	out := make([]V, 0, len(m))
	for _, k := range m.Keys() {
		out = append(out, m[k])
	}
	return out
}
