enable chromium_experimental_resource_table;

const kHouseTexture = 2u;
const kHouseSampler = 3u;

@fragment fn fs() -> @location(0) vec4f {
    let tex = getResource<texture_2d<f32>>(kHouseTexture);
    let samp = getResource<sampler>(kHouseSampler);
    return textureSample(tex, samp, vec2f(0));
}
