// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package query

import (
	"fmt"
	"io"
	"sort"
)

// Tree holds a tree structure of Query to generic Data type.
// Each separate suite, file, test of the query produces a separate tree node.
// All cases of the query produce a single leaf tree node.
type Tree[Data any] struct {
	TreeNode[Data]
}

// TreeNode is a single node in the Tree
type TreeNode[Data any] struct {
	// The full query of the node
	Query Query
	// The data associated with this node. nil is used to represent no-data.
	Data *Data
	// Children of the node. Keyed by query.Target and name.
	Children TreeNodeChildren[Data]
}

// TreeNodeChildKey is the key used by TreeNode for the Children map
type TreeNodeChildKey struct {
	// The child name. This is the string between `:` and `,` delimiters.
	// Note: that all test cases are held by a single TreeNode.
	Name string
	// The target type of the child. Examples:
	//  Query           |  Target of 'child'
	// -----------------+--------------------
	// parent:child     | Files
	// parent:x,child   | Files
	// parent:x:child   | Test
	// parent:x:y,child | Test
	// parent:x:y:child | Cases
	//
	// It's possible to have a directory and '.spec.ts' share the same name,
	// hence why we include the Target as part of the child key.
	Target Target
}

// TreeNodeChildren is a map of TreeNodeChildKey to TreeNode pointer.
// Data is the data type held by a TreeNode.
type TreeNodeChildren[Data any] map[TreeNodeChildKey]*TreeNode[Data]

// sortedChildKeys returns all the sorted children keys.
func (n *TreeNode[Data]) sortedChildKeys() []TreeNodeChildKey {
	keys := make([]TreeNodeChildKey, 0, len(n.Children))
	for key := range n.Children {
		keys = append(keys, key)
	}
	sort.Slice(keys, func(i, j int) bool {
		a, b := keys[i], keys[j]
		switch {
		case a.Name < b.Name:
			return true
		case a.Name > b.Name:
			return false
		case a.Target < b.Target:
			return true
		case a.Target > b.Target:
			return false
		}
		return false
	})
	return keys
}

// traverse performs a depth-first-search of the tree calling f for each visited
// node, starting with n, then visiting each of children in sorted order
// (pre-order traversal).
func (n *TreeNode[Data]) traverse(f func(n *TreeNode[Data]) error) error {
	if err := f(n); err != nil {
		return err
	}
	for _, key := range n.sortedChildKeys() {
		if err := n.Children[key].traverse(f); err != nil {
			return err
		}
	}
	return nil
}

// Merger is a function used to merge the children nodes of a tree.
// Merger is called with the Data of each child node. If the function returns a
// non-nil Data pointer, then this is used as the merged result. If the function
// returns nil, then the node will not be merged.
type Merger[Data any] func([]Data) *Data

// merge collapses tree nodes based on child node data, using the function f.
// merge operates on the leaf nodes first, working its way towards the root of
// the tree.
// Returns the merged target data for this node, or nil if the node is not a
// leaf and its children has non-uniform data.
func (n *TreeNode[Data]) merge(f Merger[Data]) *Data {
	// If the node is a leaf, then simply return the node's data.
	if len(n.Children) == 0 {
		return n.Data
	}

	// Build a map of child target to merged child data.
	// A nil for the value indicates that one or more children could not merge.
	mergedChildren := map[Target][]Data{}
	for key, child := range n.Children {
		// Call merge() on the child. Even if we cannot merge this node, we want
		// to do this for all children so they can merge their sub-graphs.
		childData := child.merge(f)

		if childData == nil {
			// If merge() returned nil, then the data could not be merged.
			// Mark the entire target as unmergeable.
			mergedChildren[key.Target] = nil
			continue
		}

		// Fetch the merge list for this child's target.
		list, found := mergedChildren[key.Target]
		if !found {
			// First child with the given target?
			mergedChildren[key.Target] = []Data{*childData}
			continue
		}
		if list != nil {
			mergedChildren[key.Target] = append(list, *childData)
		}
	}

	merge := func(in []Data) *Data {
		switch len(in) {
		case 0:
			return nil // nothing to merge.
		case 1:
			return &in[0] // merge of a single item results in that item
		default:
			return f(in)
		}
	}

	// Might it possible to merge this node?
	maybeMergeable := true

	// The merged data, per target
	mergedTargets := map[Target]Data{}

	// Attempt to merge each of the target's data
	for target, list := range mergedChildren {
		if list != nil { // nil == unmergeable target
			if data := merge(list); data != nil {
				// Merge success!
				mergedTargets[target] = *data
				continue
			}
		}
		maybeMergeable = false // Merge of this node is not possible
	}

	// Remove all children that have been merged
	for key := range n.Children {
		if _, merged := mergedTargets[key.Target]; merged {
			delete(n.Children, key)
		}
	}

	// Add wildcards for merged targets
	for target, data := range mergedTargets {
		data := data // Don't take address of iterator
		n.getOrCreateChild(TreeNodeChildKey{"*", target}).Data = &data
	}

	// If any of the targets are unmergeable, then we cannot merge the node itself.
	if !maybeMergeable {
		return nil
	}

	// All targets were merged. Attempt to merge each of the targets.
	data := make([]Data, 0, len(mergedTargets))
	for _, d := range mergedTargets {
		data = append(data, d)
	}
	return merge(data)
}

// print writes a textual representation of this node and its children to w.
// prefix is used as the line prefix for each node, which is appended with
// whitespace for each child node.
func (n *TreeNode[Data]) print(w io.Writer, prefix string) {
	fmt.Fprintf(w, "%v{\n", prefix)
	fmt.Fprintf(w, "%v  query: '%v'\n", prefix, n.Query)
	fmt.Fprintf(w, "%v  data:  '%v'\n", prefix, n.Data)
	for _, key := range n.sortedChildKeys() {
		n.Children[key].print(w, prefix+"  ")
	}
	fmt.Fprintf(w, "%v}\n", prefix)
}

// Format implements the io.Formatter interface.
// See https://pkg.go.dev/fmt#Formatter
func (n *TreeNode[Data]) Format(f fmt.State, verb rune) {
	n.print(f, "")
}

// getOrCreateChild returns the child with the given key if it exists,
// otherwise the child node is created and added to n and is returned.
func (n *TreeNode[Data]) getOrCreateChild(key TreeNodeChildKey) *TreeNode[Data] {
	if n.Children == nil {
		child := &TreeNode[Data]{Query: n.Query.Append(key.Target, key.Name)}
		n.Children = TreeNodeChildren[Data]{key: child}
		return child
	}
	if child, ok := n.Children[key]; ok {
		return child
	}
	child := &TreeNode[Data]{Query: n.Query.Append(key.Target, key.Name)}
	n.Children[key] = child
	return child
}

// QueryData is a pair of a Query and a generic Data type.
// Used by NewTree for constructing a tree with entries.
type QueryData[Data any] struct {
	Query Query
	Data  Data
}

// NewTree returns a new Tree populated with the given entries.
// If entries returns duplicate queries, then ErrDuplicateData will be returned.
func NewTree[Data any](entries ...QueryData[Data]) (Tree[Data], error) {
	out := Tree[Data]{}
	for _, qd := range entries {
		if err := out.Add(qd.Query, qd.Data); err != nil {
			return Tree[Data]{}, err
		}
	}
	return out, nil
}

// Add adds a new data to the tree.
// Returns ErrDuplicateData if the tree already contains a data for the given node at query
func (t *Tree[Data]) Add(q Query, d Data) error {
	node := &t.TreeNode
	q.Walk(func(q Query, t Target, n string) error {
		node = node.getOrCreateChild(TreeNodeChildKey{n, t})
		return nil
	})
	if node.Data != nil {
		return ErrDuplicateData{node.Query}
	}
	node.Data = &d
	return nil
}

// Split adds a new data to the tree, clearing any ancestor node's data.
// Returns ErrDuplicateData if the tree already contains a data for the given node at query
func (t *Tree[Data]) Split(q Query, d Data) error {
	node := &t.TreeNode
	q.Walk(func(q Query, t Target, n string) error {
		delete(node.Children, TreeNodeChildKey{Name: "*", Target: t})
		node.Data = nil
		node = node.getOrCreateChild(TreeNodeChildKey{n, t})
		return nil
	})
	if node.Data != nil {
		return ErrDuplicateData{node.Query}
	}
	node.Data = &d
	return nil
}

// GetOrCreate returns existing, or adds a new data to the tree.
func (t *Tree[Data]) GetOrCreate(q Query, create func() Data) *Data {
	node := &t.TreeNode
	q.Walk(func(q Query, t Target, n string) error {
		node = node.getOrCreateChild(TreeNodeChildKey{n, t})
		return nil
	})
	if node.Data == nil {
		data := create()
		node.Data = &data
	}
	return node.Data
}

// Reduce reduces the tree using the Merger function f.
// If the Merger function returns a non-nil Data value, then this will be used
// to replace the non-leaf node with a new leaf node holding the returned Data.
// This process recurses up to the tree root.
func (t *Tree[Data]) Reduce(f Merger[Data]) {
	for _, root := range t.TreeNode.Children {
		root.merge(f)
	}
}

// ReduceUnder reduces the sub-tree under the given query using the Merger
// function f.
// If the Merger function returns a non-nil Data value, then this will be used
// to replace the non-leaf node with a new leaf node holding the returned Data.
// This process recurses up to the node pointed at by the query to.
func (t *Tree[Data]) ReduceUnder(to Query, f Merger[Data]) error {
	node := &t.TreeNode
	return to.Walk(func(q Query, t Target, n string) error {
		if n == "*" {
			node.merge(f)
			return nil
		}
		child, ok := node.Children[TreeNodeChildKey{n, t}]
		if !ok {
			return ErrNoDataForQuery{q}
		}
		node = child
		if q == to {
			node.merge(f)
		}
		return nil
	})
}

// glob calls f for every node under the given query.
func (t *Tree[Data]) glob(fq Query, f func(f *TreeNode[Data]) error) error {
	node := &t.TreeNode
	return fq.Walk(func(q Query, t Target, n string) error {
		if n == "*" {
			// Wildcard reached.
			// Glob the parent, but restrict to the wildcard target type.
			for _, key := range node.sortedChildKeys() {
				child := node.Children[key]
				if child.Query.Target() == t {
					if err := child.traverse(f); err != nil {
						return err
					}
				}
			}
			return nil
		}
		switch t {
		case Suite, Files, Tests:
			child, ok := node.Children[TreeNodeChildKey{n, t}]
			if !ok {
				return ErrNoDataForQuery{q}
			}
			node = child
		case Cases:
			for _, key := range node.sortedChildKeys() {
				child := node.Children[key]
				if child.Query.Contains(fq) {
					if err := f(child); err != nil {
						return err
					}
				}
			}
			return nil
		}
		if q == fq {
			return node.traverse(f)
		}
		return nil
	})
}

// Replace replaces the sub-tree matching the query 'what' with the Data 'with'
func (t *Tree[Data]) Replace(what Query, with Data) error {
	node := &t.TreeNode
	return what.Walk(func(q Query, t Target, n string) error {
		childKey := TreeNodeChildKey{n, t}
		if q == what {
			for key, child := range node.Children {
				// Use Query.Contains() to handle matching of Cases
				// (which are not split into tree nodes)
				if q.Contains(child.Query) {
					delete(node.Children, key)
				}
			}
			node = node.getOrCreateChild(childKey)
			node.Data = &with
		} else {
			child, ok := node.Children[childKey]
			if !ok {
				return ErrNoDataForQuery{q}
			}
			node = child
		}
		return nil
	})
}

// List returns the tree nodes flattened as a list of QueryData
func (t *Tree[Data]) List() []QueryData[Data] {
	out := []QueryData[Data]{}
	t.traverse(func(n *TreeNode[Data]) error {
		if n.Data != nil {
			out = append(out, QueryData[Data]{n.Query, *n.Data})
		}
		return nil
	})
	return out
}

// Glob returns a list of QueryData's for every node that is under the given
// query, which holds data.
// Glob handles wildcards as well as non-wildcard queries:
//   - A non-wildcard query will match the node itself, along with every node
//     under the query. For example: 'a:b' will match every File and Test
//     node under 'a:b', including 'a:b' itself.
//   - A wildcard Query will include every node under the parent node with the
//     matching Query target. For example: 'a:b:*' will match every Test
//     node (excluding File nodes) under 'a:b', 'a:b' will not be included.
func (t *Tree[Data]) Glob(q Query) ([]QueryData[Data], error) {
	out := []QueryData[Data]{}
	err := t.glob(q, func(n *TreeNode[Data]) error {
		if n.Data != nil {
			out = append(out, QueryData[Data]{n.Query, *n.Data})
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	return out, nil
}
