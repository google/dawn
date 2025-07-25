@group(0) @binding(1) var x_108 : sampler;

@group(0) @binding(0) var x_109 : texture_depth_2d_array;

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
