fn i(x : i32) -> i32 {
  return -(x);
}

fn vi(x : vec4<i32>) -> vec4<i32> {
  return -(x);
}

@compute @workgroup_size(1)
fn main() {
  _ = i(1);
  _ = vi(vec4i());
}
