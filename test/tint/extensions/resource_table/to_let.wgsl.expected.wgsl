enable chromium_experimental_resource_table;

const kHouseTexture = 2u;

@fragment
fn fs() {
  let tex = getResource<texture_1d<f32>>(kHouseTexture);
  let texture_load = textureLoad(tex, 0, 0);
}
