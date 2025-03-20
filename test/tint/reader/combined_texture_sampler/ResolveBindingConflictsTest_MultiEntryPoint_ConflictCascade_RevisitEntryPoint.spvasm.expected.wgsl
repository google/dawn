struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

@group(0) @binding(0) var x_100 : texture_2d<f32>;

@group(0) @binding(1) var x_101 : sampler;

@group(0) @binding(2) var<uniform> x_102 : st_ty;

@group(0) @binding(3) var<storage, read_write> x_103 : st_ty;

@group(0) @binding(3) var x_2005 : sampler;

@group(0) @binding(3) var x_2006 : texture_2d<f32>;

@group(0) @binding(4) var x_105 : sampler;

fn alpha_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn alpha() {
  alpha_1();
}

fn beta_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn beta() {
  beta_1();
}
