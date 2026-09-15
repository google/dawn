@group(0) @binding(0)
var<storage> v : array<i32, 65535>;

struct A {
  a : array<f32, 65535>,
}
@group(0) @binding(1)
var<storage> b : A;

@compute @workgroup_size(1)
fn main() {
  _ = v;
  _ = b;
}
