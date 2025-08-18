@group(0u) @binding(0u) var v : texture_2d<f32>;

@group(0u) @binding(1u) var v_1 : sampler;

struct st_ty {
  tint_symbol : u32,
}

@group(0u) @binding(0u) var<uniform> v_2 : st_ty;

@group(0u) @binding(0u) var<storage, read_write> v_3 : st_ty;

@group(0u) @binding(2u) var v_4 : sampler;

@group(0u) @binding(0u) var v_5 : texture_2d<f32>;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
}
