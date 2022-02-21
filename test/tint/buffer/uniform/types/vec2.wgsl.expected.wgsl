@group(0) @binding(0) var<uniform> u : vec2<i32>;

@stage(compute) @workgroup_size(1)
fn main() {
  let x = u;
}
