@group(0u) @binding(0u) var v : texture_2d<f32>;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  _ = v;
}
