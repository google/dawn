enable chromium_experimental_resource_table;

const kHouseTexture = 4u;

@fragment
fn fs() {
  let t = hasResource<texture_2d<i32>>(kHouseTexture);
}
