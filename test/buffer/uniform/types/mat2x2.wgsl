@group(0) @binding(0)
var<uniform> u : mat2x2<f32>;

@stage(compute) @workgroup_size(1)
fn main() {
  let x = u;
}
