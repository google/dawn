enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var sampled_textures : resource_binding;

@fragment fn fs() {
    let t1d = textureLoad(getBinding<texture_1d<f32>>(sampled_textures, 2), 0, 0);
    let t2d = textureLoad(getBinding<texture_2d<i32>>(sampled_textures, 1), vec2(0, 1), 0);
    let tcube = textureLoad(getBinding<texture_3d<u32>>(sampled_textures, 1), vec3(2, 1, 0), 0);
}
