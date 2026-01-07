enable chromium_experimental_resource_table;

@group(1) @binding(0) var<uniform> index : u32;

@fragment
fn fs() {
  let texture_load = textureLoad(getResource<texture_3d<f32>>(index), vec3(0, 0, 0), 0);
}
