struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

@group(0) @binding(0) var x_100 : texture_2d<f32>;

@group(0) @binding(1) var x_101 : sampler;

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
