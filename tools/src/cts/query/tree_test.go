package query_test

import (
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"github.com/google/go-cmp/cmp"
)

var (
	abort   = "Abort"
	crash   = "Crash"
	failure = "Failure"
	pass    = "Pass"
	skip    = "Skip"
)

func NewTree[Data any](t *testing.T, entries ...query.QueryData[Data]) (query.Tree[Data], error) {
	return query.NewTree(entries...)
}

func TestNewSingle(t *testing.T) {
	type Tree = query.Tree[string]
	type Node = query.TreeNode[string]
	type QueryData = query.QueryData[string]
	type Children = query.TreeNodeChildren[string]

	type Test struct {
		in     QueryData
		expect Tree
	}
	for _, test := range []Test{
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`*`, query.Files}: {
									Query: Q(`suite:*`),
									Data:  &pass,
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`*`, query.Files}: {
											Query: Q(`suite:a,*`),
											Data:  &pass,
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`*`, query.Tests}: {
													Query: Q(`suite:a,b:*`),
													Data:  &pass,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:c:*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Tests}: {
													Query: Q(`suite:a,b:c`),
													Children: Children{
														query.TreeNodeChildKey{`*`, query.Cases}: {
															Query: Q(`suite:a,b:c:*`),
															Data:  &pass,
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b,c:d,e:f="g";h=[1,2,3];i=4;*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Files}: {
													Query: Q(`suite:a,b,c`),
													Children: Children{
														query.TreeNodeChildKey{`d`, query.Tests}: {
															Query: Q(`suite:a,b,c:d`),
															Children: Children{
																query.TreeNodeChildKey{`e`, query.Tests}: {
																	Query: Q(`suite:a,b,c:d,e`),
																	Children: Children{
																		query.TreeNodeChildKey{`f="g";h=[1,2,3];i=4;*`, query.Cases}: {
																			Query: Q(`suite:a,b,c:d,e:f="g";h=[1,2,3];i=4;*`),
																			Data:  &pass,
																		},
																	},
																},
															},
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:c:d="e";*`), Data: pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Tests}: {
													Query: Q(`suite:a,b:c`),
													Children: Children{
														query.TreeNodeChildKey{`d="e";*`, query.Cases}: {
															Query: Q(`suite:a,b:c:d="e";*`),
															Data:  &pass,
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	} {
		got, err := NewTree(t, test.in)
		if err != nil {
			t.Errorf("NewTree(%v): %v", test.in, err)
			continue
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("NewTree(%v) tree was not as expected:\n%v", test.in, diff)
		}
	}
}

func TestNewMultiple(t *testing.T) {
	type Tree = query.Tree[string]
	type Node = query.TreeNode[string]
	type QueryData = query.QueryData[string]
	type Children = query.TreeNodeChildren[string]

	got, err := NewTree(t,
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	expect := Tree{
		TreeNode: Node{
			Children: Children{
				query.TreeNodeChildKey{`suite`, query.Suite}: {
					Query: Q(`suite`),
					Children: Children{
						query.TreeNodeChildKey{`a`, query.Files}: {
							Query: Q(`suite:a`),
							Children: Children{
								query.TreeNodeChildKey{`b`, query.Files}: {
									Query: Q(`suite:a,b`),
									Children: Children{
										query.TreeNodeChildKey{`c`, query.Tests}: {
											Query: Q(`suite:a,b:c`),
											Children: Children{
												query.TreeNodeChildKey{`d="e";*`, query.Cases}: {
													Query: Q(`suite:a,b:c:d="e";*`),
													Data:  &failure,
												},
												query.TreeNodeChildKey{`f="g";*`, query.Cases}: {
													Query: Q(`suite:a,b:c:f="g";*`),
													Data:  &skip,
												},
											},
										},
									},
								},
							},
						},
						query.TreeNodeChildKey{`h`, query.Files}: {
							Query: query.Query{
								Suite: `suite`,
								Files: `h`,
							},
							Children: Children{
								query.TreeNodeChildKey{`b`, query.Files}: {
									Query: query.Query{
										Suite: `suite`,
										Files: `h,b`,
									},
									Children: Children{
										query.TreeNodeChildKey{`c`, query.Tests}: {
											Query: query.Query{
												Suite: `suite`,
												Files: `h,b`,
												Tests: `c`,
											},
											Children: Children{
												query.TreeNodeChildKey{`f="g";*`, query.Cases}: {
													Query: query.Query{
														Suite: `suite`,
														Files: `h,b`,
														Tests: `c`,
														Cases: `f="g";*`,
													},
													Data: &abort,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	}
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("NewTree() was not as expected:\n%v", diff)
		t.Errorf("got:\n%v", got)
		t.Errorf("expect:\n%v", expect)
	}
}

func TestNewWithCollision(t *testing.T) {
	type Tree = query.Tree[string]
	type QueryData = query.QueryData[string]

	got, err := NewTree(t,
		QueryData{Query: Q(`suite:a,b:c:*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:*`), Data: skip},
	)
	expect := Tree{}
	expectErr := query.ErrDuplicateData{
		Query: Q(`suite:a,b:c:*`),
	}
	if diff := cmp.Diff(err, expectErr); diff != "" {
		t.Errorf("NewTree() error was not as expected:\n%v", diff)
	}
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("NewTree() was not as expected:\n%v", diff)
	}
}

func TestSplit(t *testing.T) {
	type Tree = query.Tree[string]
	type Node = query.TreeNode[string]
	type QueryData = query.QueryData[string]
	type Children = query.TreeNodeChildren[string]

	type Test struct {
		in   QueryData
		pre  Tree
		post Tree
	}
	for _, test := range []Test{
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:*`),
				Data:  pass,
			},
			post: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`*`, query.Files}: {
									Query: Q(`suite:*`),
									Data:  &pass,
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:*`),
				Data:  pass,
			},
			pre: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`*`, query.Files}: {
											Query: Q(`suite:a,*`),
											Data:  &pass,
										},
									},
								},
							},
						},
					},
				},
			},
			post: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`*`, query.Tests}: {
													Query: Q(`suite:a,b:*`),
													Data:  &pass,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a:*`),
				Data:  pass,
			},
			pre: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`*`, query.Files}: {
													Query: Q(`suite:a,b,*`),
													Data:  &pass,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
			post: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`*`, query.Tests}: {
											Query: Q(`suite:a:*`),
											Data:  &pass,
										},
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`*`, query.Files}: {
													Query: Q(`suite:a,b,*`),
													Data:  &pass,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:c:*`),
				Data:  pass,
			},
			pre: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`*`, query.Tests}: {
													Query: Q(`suite:a,b:*`),
													Data:  &pass,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
			post: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Tests}: {
													Query: Q(`suite:a,b:c`),
													Children: Children{
														query.TreeNodeChildKey{`*`, query.Cases}: {
															Query: Q(`suite:a,b:c:*`),
															Data:  &pass,
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	} {
		tree := test.pre
		if err := tree.Split(test.in.Query, test.in.Data); err != nil {
			t.Errorf("NewTree(%v): %v", test.in, err)
			continue
		}
		if diff := cmp.Diff(tree, test.post); diff != "" {
			t.Errorf("Split(%v) tree was not as expected:\n%v", test.in, diff)
		}
	}
}

func TestList(t *testing.T) {
	type QueryData = query.QueryData[string]

	tree, err := NewTree(t,
		QueryData{Query: Q(`suite:*`), Data: skip},
		QueryData{Query: Q(`suite:a,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d;*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	got := tree.List()
	expect := []QueryData{
		{Query: Q(`suite:*`), Data: skip},
		{Query: Q(`suite:a,*`), Data: failure},
		{Query: Q(`suite:a,b,*`), Data: failure},
		{Query: Q(`suite:a,b:c:*`), Data: failure},
		{Query: Q(`suite:a,b:c:d;*`), Data: failure},
		{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
		{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
	}
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("List() was not as expected:\n%v", diff)
	}
}

// reducer is used by Reduce() and ReduceUnder() tests for reducing the tree.
// reducer returns a pointer to the common string if all strings in data are
// equal, otherwise returns nil
func reducer(data []string) *string {
	if s := container.NewSet(data...); len(s) == 1 {
		item := s.One()
		return &item
	}
	return nil
}

func TestReduce(t *testing.T) {
	type QueryData = query.QueryData[string]

	type Test struct {
		name   string
		in     []QueryData
		expect []QueryData
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			name: "Different file results - A",
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Different file results - B",
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
				{Query: Q(`suite:a,d,*`), Data: skip},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
				{Query: Q(`suite:a,d,*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Different test results",
			in: []QueryData{
				{Query: Q(`suite:a,b:*`), Data: failure},
				{Query: Q(`suite:a,c:*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b:*`), Data: failure},
				{Query: Q(`suite:a,c:*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Same file results",
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: failure},
			},
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Same test results",
			in: []QueryData{
				{Query: Q(`suite:a,b:*`), Data: failure},
				{Query: Q(`suite:a,c:*`), Data: failure},
			},
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "File vs test",
			in: []QueryData{
				{Query: Q(`suite:a:b,c*`), Data: failure},
				{Query: Q(`suite:a,b,c*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: pass},
				{Query: Q(`suite:a:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Sibling cases, no reduce",
			in: []QueryData{
				{Query: Q(`suite:a:b:c;d=e;f=g;*`), Data: failure},
				{Query: Q(`suite:a:b:c;d=e;f=h;*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a:b:c;d=e;f=g;*`), Data: failure},
				{Query: Q(`suite:a:b:c;d=e;f=h;*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Sibling cases, reduce to test",
			in: []QueryData{
				{Query: Q(`suite:a:b:c=1;d="x";*`), Data: failure},
				{Query: Q(`suite:a:b:c=1;d="y";*`), Data: failure},
				{Query: Q(`suite:a:z:*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a:b:*`), Data: failure},
				{Query: Q(`suite:a:z:*`), Data: pass},
			},
		},
	} {
		tree, err := NewTree(t, test.in...)
		if err != nil {
			t.Errorf("Test '%v':\nNewTree() returned %v", test.name, err)
			continue
		}
		tree.Reduce(reducer)
		results := tree.List()
		if diff := cmp.Diff(results, test.expect); diff != "" {
			t.Errorf("Test '%v':\n%v", test.name, diff)
		}
	}
}

func TestReduceUnder(t *testing.T) {
	type QueryData = query.QueryData[string]

	type Test struct {
		location  string
		to        query.Query
		in        []QueryData
		expect    []QueryData
		expectErr error
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:a,b,*`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:a,*`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:*`),
			in: []QueryData{
				{Query: Q(`suite:a,b:*`), Data: failure},
			},
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:a,*`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:a,*`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: pass},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:a`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: pass},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:x`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: pass},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: pass},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			expectErr: query.ErrNoDataForQuery{
				Query: Q(`suite:x`),
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			to:       Q(`suite:a,b,c,*`),
			in: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: pass},
			},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: pass},
			},
			expectErr: query.ErrNoDataForQuery{
				Query: Q(`suite:a,b,c`),
			},
		},
	} {
		tree, err := NewTree(t, test.in...)
		if err != nil {
			t.Errorf("\n%v NewTree(): %v", test.location, err)
			continue
		}
		err = tree.ReduceUnder(test.to, reducer)
		if diff := cmp.Diff(err, test.expectErr); diff != "" {
			t.Errorf("\n%v ReduceUnder(): %v", test.location, err)
		}
		results := tree.List()
		if diff := cmp.Diff(results, test.expect); diff != "" {
			t.Errorf("\n%v List(): %v", test.location, diff)
		}
	}
}

func TestReplace(t *testing.T) {
	type QueryData = query.QueryData[string]

	type Test struct {
		name        string
		base        []QueryData
		replacement QueryData
		expect      []QueryData
		expectErr   error
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			name: "Replace file. Direct",
			base: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			replacement: QueryData{Q(`suite:a,b,*`), skip},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: skip},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Replace file. Indirect",
			base: []QueryData{
				{Query: Q(`suite:a,b,c,*`), Data: failure},
				{Query: Q(`suite:a,b,d,*`), Data: pass},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
			replacement: QueryData{Q(`suite:a,b,*`), skip},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: skip},
				{Query: Q(`suite:a,c,*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "File vs Test",
			base: []QueryData{
				{Query: Q(`suite:a,b:c,*`), Data: crash},
				{Query: Q(`suite:a,b:d,*`), Data: abort},
				{Query: Q(`suite:a,b,c,*`), Data: failure},
				{Query: Q(`suite:a,b,d,*`), Data: pass},
			},
			replacement: QueryData{Q(`suite:a,b,*`), skip},
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Cases. * with *",
			base: []QueryData{
				{Query: Q(`suite:file:test:*`), Data: failure},
			},
			replacement: QueryData{Q(`suite:file:test:*`), pass},
			expect: []QueryData{
				{Query: Q(`suite:file:test:*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Cases. Mixed with *",
			base: []QueryData{
				{Query: Q(`suite:file:test:a=1,*`), Data: failure},
				{Query: Q(`suite:file:test:a=2,*`), Data: skip},
				{Query: Q(`suite:file:test:a=3,*`), Data: crash},
			},
			replacement: QueryData{Q(`suite:file:test:*`), pass},
			expect: []QueryData{
				{Query: Q(`suite:file:test:*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Cases. Replace partial - (a=1)",
			base: []QueryData{
				{Query: Q(`suite:file:test:a=1;b=x;*`), Data: failure},
				{Query: Q(`suite:file:test:a=1;b=y;*`), Data: failure},
				{Query: Q(`suite:file:test:a=2;b=y;*`), Data: failure},
			},
			replacement: QueryData{Q(`suite:file:test:a=1;*`), pass},
			expect: []QueryData{
				{Query: Q(`suite:file:test:a=1;*`), Data: pass},
				{Query: Q(`suite:file:test:a=2;b=y;*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Cases. Replace partial - (b=y)",
			base: []QueryData{
				{Query: Q(`suite:file:test:a=1;b=x;*`), Data: failure},
				{Query: Q(`suite:file:test:a=1;b=y;*`), Data: failure},
				{Query: Q(`suite:file:test:a=2;b=y;*`), Data: failure},
			},
			replacement: QueryData{Q(`suite:file:test:b=y;*`), pass},
			expect: []QueryData{
				{Query: Q(`suite:file:test:a=1;b=x;*`), Data: failure},
				{Query: Q(`suite:file:test:b=y;*`), Data: pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Error. No data for query - short",
			base: []QueryData{
				{Query: Q(`suite:file:test:a=1;b=x;*`), Data: failure},
			},
			replacement: QueryData{Q(`suite:missing:*`), pass},
			expect: []QueryData{
				{Query: Q(`suite:file:test:a=1;b=x;*`), Data: failure},
			},
			expectErr: query.ErrNoDataForQuery{Q(`suite:missing`)},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "Error. No data for query - long",
			base: []QueryData{
				{Query: Q(`suite:file:test:*`), Data: failure},
			},
			replacement: QueryData{Q(`suite:file:test,missing,*`), pass},
			expect: []QueryData{
				{Query: Q(`suite:file:test:*`), Data: failure},
			},
			expectErr: query.ErrNoDataForQuery{Q(`suite:file:test,missing`)},
		},
	} {
		tree, err := NewTree(t, test.base...)
		if err != nil {
			t.Errorf("Test '%v':\nNewTree(): %v", test.name, err)
			continue
		}
		err = tree.Replace(test.replacement.Query, test.replacement.Data)
		if diff := cmp.Diff(err, test.expectErr); diff != "" {
			t.Errorf("Test '%v':\nReplace() error: %v", test.name, err)
			continue
		}
		if diff := cmp.Diff(tree.List(), test.expect); diff != "" {
			t.Errorf("Test '%v':\n%v", test.name, diff)
		}
	}
}

func TestGlob(t *testing.T) {
	type QueryData = query.QueryData[string]

	tree, err := NewTree(t,
		QueryData{Query: Q(`suite:*`), Data: skip},
		QueryData{Query: Q(`suite:a,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d;*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
		QueryData{Query: Q(`suite:a,b:d:*`), Data: failure},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	type Test struct {
		query     query.Query
		expect    []QueryData
		expectErr error
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite`),
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: skip},
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
				{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:*`),
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: skip},
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
				{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a`),
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,*`),
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b`),
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b,*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:d="e";*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:d;*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:f="g";*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query:     Q(`suite:x,y`),
			expectErr: query.ErrNoDataForQuery{Q(`suite:x`)},
		},
		{ //////////////////////////////////////////////////////////////////////
			query:     Q(`suite:a,b:x`),
			expectErr: query.ErrNoDataForQuery{Q(`suite:a,b:x`)},
		},
	} {
		got, err := tree.Glob(test.query)
		if diff := cmp.Diff(err, test.expectErr); diff != "" {
			t.Errorf("Glob('%v') error: %v", test.query, err)
			continue
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Glob('%v'):\n%v", test.query, diff)
		}
	}
}

func TestFormat(t *testing.T) {
	type QueryData = query.QueryData[string]

	tree, err := NewTree(t,
		QueryData{Query: Q(`suite:*`), Data: skip},
		QueryData{Query: Q(`suite:a,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d;*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
		QueryData{Query: Q(`suite:a,b:d:*`), Data: failure},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	callA := fmt.Sprint(tree)
	callB := fmt.Sprint(tree)

	if diff := cmp.Diff(callA, callB); diff != "" {
		t.Errorf("Format():\n%v", diff)
	}
}
