@group(0) @binding(1) var orig_var_sampler : sampler;

@group(0) @binding(0) var orig_var_image : texture_2d<f32>;

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
