enable chromium_experimental_dynamic_binding;

const kHouseTexture = 2u;

@fragment fn fs() {
    let texture_load = textureLoad(getResource<texture_1d<f32>>(kHouseTexture), 0, 0);
}
