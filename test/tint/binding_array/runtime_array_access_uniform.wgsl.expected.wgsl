enable chromium_experimental_dynamic_binding;

@group(1) @binding(0) var<uniform> index : u32;

@group(0) @binding(0) var sampled_textures : binding_array<texture_2d<f32>>;

@fragment
fn fs() {
  let texture_load = textureLoad(sampled_textures[index], vec2(0, 0), 0);
}
