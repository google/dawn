struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

@group(0) @binding(1) var x_100 : sampler;

@group(0) @binding(0) var x_101 : texture_2d<f32>;

@group(0) @binding(2) var x_102 : texture_2d<f32>;

@group(0) @binding(3) var x_103 : sampler;

@group(0) @binding(4) var<uniform> x_104 : st_ty;

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
