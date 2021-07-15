[[block]]
struct Constants {
  zero : u32;
};

[[group(1), binding(0)]] var<uniform> constants : Constants;

[[block]]
struct Result {
  value : u32;
};

[[group(1), binding(1)]] var<storage, write> result : Result;

struct S {
  data : array<u32, 3>;
};

var<private> s : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  s.data[constants.zero] = 0u;
}
