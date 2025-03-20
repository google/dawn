struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

@group(0) @binding(1) var x_100 : sampler;

@group(0) @binding(0) var x_101 : texture_2d<f32>;

@group(0) @binding(2) var<uniform> x_102 : st_ty;

@group(0) @binding(2) var<storage, read_write> x_103 : st_ty;

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

fn gamma_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn gamma() {
  gamma_1();
}
