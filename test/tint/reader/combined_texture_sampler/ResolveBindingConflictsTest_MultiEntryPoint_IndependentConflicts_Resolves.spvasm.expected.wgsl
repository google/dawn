@group(0u) @binding(2u) var v : sampler;

@group(0u) @binding(0u) var v_1 : texture_2d<f32>;

@group(0u) @binding(0u) var v_2 : texture_2d<f32>;

@group(0u) @binding(3u) var v_3 : sampler;

@compute @workgroup_size(1u, 1u, 1u)
fn alpha() {
}

@compute @workgroup_size(1u, 1u, 1u)
fn beta() {
}
