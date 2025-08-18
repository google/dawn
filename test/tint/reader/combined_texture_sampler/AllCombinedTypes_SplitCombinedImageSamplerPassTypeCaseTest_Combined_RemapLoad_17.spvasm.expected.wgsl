@group(0u) @binding(1u) var v : sampler;

@group(0u) @binding(0u) var v_1 : texture_3d<i32>;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  _ = v_1;
  _ = v;
}
