// Copyright 2021 The Tint Authors.
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

package substr

import (
	"strings"
	"testing"
)

func TestFixSubstr(t *testing.T) {
	type test struct {
		body   string
		substr string
		expect string
	}

	for _, test := range []test{
		{
			body:   "abc_def_ghi_jkl_mno",
			substr: "def_XXX_jkl",
			expect: "def_ghi_jkl",
		},
		{
			body:   "abc\ndef\nghi\njkl\nmno",
			substr: "def\nXXX\njkl",
			expect: "def\nghi\njkl",
		},
		{
			body:   "aaaaa12345ccccc",
			substr: "1x345",
			expect: "12345",
		},
		{
			body:   "aaaaa12345ccccc",
			substr: "12x45",
			expect: "12345",
		},
		{
			body:   "aaaaa12345ccccc",
			substr: "123x5",
			expect: "12345",
		},
		{
			body:   "aaaaaaaaaaaaa",
			substr: "bbbbbbbbbbbbb",
			expect: "", // cannot produce a sensible diff
		}, { ///////////////////////////////////////////////////////////////////
			body: `Return{
  {
    ScalarInitializer[not set]{42u}
  }
}
`,
			substr: `Return{
  {
    ScalarInitializer[not set]{42}
  }
}`,
			expect: `Return{
  {
    ScalarInitializer[not set]{42u}
  }
}`,
		}, { ///////////////////////////////////////////////////////////////////
			body: `VariableDeclStatement{
  Variable{
    x_1
    function
    __u32
  }
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{42u}
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{0u}
}
Return{}
`,
			substr: `Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{42}
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{0}
}`,
			expect: `Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{42u}
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{0u}
}`,
		}, { ///////////////////////////////////////////////////////////////////
			body: `VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarInitializer[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarInitializer[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarInitializer[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarInitializer[not set]{1u}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarInitializer[not set]{1.500000}
    }
  }
}
`,
			substr: `VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarInitializer[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarInitializer[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarInitializer[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarInitializer[not set]{1}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarInitializer[not set]{1.500000}
    }
  }
}
`,
			expect: `VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarInitializer[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarInitializer[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarInitializer[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarInitializer[not set]{1u}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarInitializer[not set]{1.500000}
    }
  }
}
`,
		},
	} {
		body := strings.ReplaceAll(test.body, "\n", "␤")
		substr := strings.ReplaceAll(test.substr, "\n", "␤")
		expect := strings.ReplaceAll(test.expect, "\n", `␤`)
		got := strings.ReplaceAll(Fix(test.body, test.substr), "\n", "␤")
		if got != expect {
			t.Errorf("Test failure:\nbody:   '%v'\nsubstr: '%v'\nexpect: '%v'\ngot:    '%v'\n\n", body, substr, expect, got)
		}

	}
}
