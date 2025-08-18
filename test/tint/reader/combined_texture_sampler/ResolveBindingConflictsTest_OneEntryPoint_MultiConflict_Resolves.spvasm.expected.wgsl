@group(0u) @binding(1u) var v : sampler;

@group(0u) @binding(0u) var v_1 : texture_2d<f32>;

@group(0u) @binding(2u) var v_2 : texture_2d<f32>;

@group(0u) @binding(3u) var v_3 : sampler;

struct st_ty {
  tint_symbol : u32,
}

@group(0u) @binding(4u) var<uniform> v_4 : st_ty;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  let v_5 = &(v_4);
}
