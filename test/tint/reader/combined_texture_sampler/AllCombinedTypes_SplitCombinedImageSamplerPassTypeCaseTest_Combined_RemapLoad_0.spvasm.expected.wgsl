@group(0) @binding(1) var x_104 : sampler;

@group(0) @binding(0) var x_105 : texture_2d<f32>;

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
