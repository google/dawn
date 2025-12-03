enable chromium_experimental_resource_table;

const kHouseTexture = 0u;
const kMouseTexture = 1i;
const kCatTexture = 2i;

@fragment fn fs() {
    let t1d = textureLoad(getResource<texture_1d<f32>>(kHouseTexture), 0, 0);
    let t2d = textureLoad(getResource<texture_2d<i32>>(kMouseTexture), vec2(0, 1), 0);
    let tcube = textureLoad(getResource<texture_3d<u32>>(kCatTexture), vec3(2, 1, 0), 0);
}
