enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var sampled_textures : binding_array<texture_2d<f32>>;

@fragment
fn fs() {
  let texture_load = textureLoad(sampled_textures[0], vec2(0, 0), 0);
}
