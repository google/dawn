enable f16;

@group(0) @binding(0) var<uniform> u : mat4x3<f16>;

@compute @workgroup_size(1)
fn main() {
  let x = u;
}
