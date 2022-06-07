@group(0) @binding(0) var<uniform> u : array<vec4<f32>, 4>;

@compute @workgroup_size(1)
fn main() {
  let x = u;
}
