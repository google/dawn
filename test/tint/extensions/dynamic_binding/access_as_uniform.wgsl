enable chromium_experimental_dynamic_binding;

@group(1) @binding(0) var<uniform> index : u32;
@group(0) @binding(0) var sampled_textures : resource_binding;

@fragment fn fs() {
    let t = getBinding<texture_3d<f32>>(sampled_textures, index);
    let texture_load = textureLoad(t, vec2(0, 0), 0);
}
