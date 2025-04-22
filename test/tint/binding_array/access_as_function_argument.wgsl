@group(0) @binding(0) var sampled_textures : binding_array<texture_2d<f32>, 4>;

@fragment fn fs() {
    do_texture_load(sampled_textures[0]);
}

fn do_texture_load(t : texture_2d<f32>) {
    let texture_load = textureLoad(t, vec2(0, 0), 0);
}
