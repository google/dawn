@group(0u) @binding(1u) var v : sampler;

@group(0u) @binding(0u) var v_1 : texture_2d<f32>;

@group(0u) @binding(2u) var v_2 : texture_2d<f32>;

@group(0u) @binding(3u) var v_3 : sampler;

struct st_ty {
  tint_symbol : u32,
}

@group(0u) @binding(4u) var<uniform> v_4 : st_ty;

fn v_5() {
}

fn v_6() {
}

fn v_7() {
  v_5();
  v_6();
}

fn v_8() {
  v_7();
  v_6();
}

fn v_9() {
  v_6();
  v_8();
  let v_10 = &(v_4);
}

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  v_9();
}
