@group(0u) @binding(1u) var v : sampler;

@group(0u) @binding(0u) var v_1 : texture_2d<f32>;

struct st_ty {
  tint_symbol : u32,
}

@group(0u) @binding(2u) var<uniform> v_2 : st_ty;

@group(0u) @binding(2u) var<storage, read_write> v_3 : st_ty;

@compute @workgroup_size(1u, 1u, 1u)
fn alpha() {
}

@compute @workgroup_size(1u, 1u, 1u)
fn beta() {
  let v_4 = &(v_2);
}

@compute @workgroup_size(1u, 1u, 1u)
fn gamma() {
  let v_5 = &(v_3);
}
