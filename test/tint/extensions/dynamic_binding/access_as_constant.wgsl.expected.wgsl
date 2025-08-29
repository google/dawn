enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var sampled_textures : resource_binding;

@fragment
fn fs() {
  let texture_load = textureLoad(getBinding<texture_1d<f32>>(sampled_textures, 2), 0, 0);
}
