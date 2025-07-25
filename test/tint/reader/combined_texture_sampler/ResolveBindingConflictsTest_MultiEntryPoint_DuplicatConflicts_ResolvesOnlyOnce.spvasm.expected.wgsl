struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

@group(0) @binding(1) var x_100 : sampler;

@group(0) @binding(0) var x_101 : texture_2d<f32>;

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
