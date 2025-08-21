enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var sampled_textures : binding_array<texture_2d<f32>>;

@fragment
fn fs() {
  do_texture_load(sampled_textures);
}

fn do_texture_load(ts : binding_array<texture_2d<f32>>) {
  let texture_load = textureLoad(ts[0], vec2(0, 0), 0);
}
