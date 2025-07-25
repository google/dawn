@group(0) @binding(0) var<storage, read_write> s : array<u32>;

struct A {
  b : vec2u,
  c : u32,
}

@compute @workgroup_size(1u)
fn main() {
  const kArray = array(A(vec2u(1, 2), 3), A(vec2u(4, 5), 6));
  var q = 0u;
  s[0] = kArray[q].b.x;
}
