enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var sampled_textures : resource_binding;

@fragment
fn fs() {
  let t = hasBinding<texture_2d<i32>>(sampled_textures, 4);
}
