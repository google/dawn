@group(0u) @binding(1u) var orig_var_sampler : sampler;

@group(0u) @binding(0u) var orig_var_image : texture_2d<f32>;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  _ = orig_var_image;
  _ = orig_var_sampler;
}
