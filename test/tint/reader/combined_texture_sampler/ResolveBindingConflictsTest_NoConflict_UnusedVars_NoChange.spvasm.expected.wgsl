struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

@group(0) @binding(0) var x_100 : texture_2d<f32>;

@group(0) @binding(0) var x_101 : sampler;

@group(0) @binding(0) var<uniform> x_102 : st_ty;

@group(0) @binding(0) var<storage, read_write> x_103 : st_ty;

@group(0) @binding(0) var x_105 : sampler;

@group(0) @binding(0) var x_106 : texture_2d<f32>;

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
