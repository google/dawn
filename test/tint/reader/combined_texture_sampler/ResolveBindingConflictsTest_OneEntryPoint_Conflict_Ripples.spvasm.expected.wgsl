@group(0u) @binding(1u) var v : sampler;

@group(0u) @binding(0u) var v_1 : texture_2d<f32>;

struct st_ty {
  tint_symbol : u32,
}

@group(0u) @binding(2u) var<uniform> v_2 : st_ty;

@group(0u) @binding(3u) var<storage, read_write> v_3 : st_ty;

@group(0u) @binding(4u) var v_4 : sampler;

@group(0u) @binding(3u) var v_5 : texture_2d<f32>;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  let v_6 = &(v_2);
  let v_7 = &(v_3);
}
