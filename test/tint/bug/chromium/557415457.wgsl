enable chromium_experimental_resource_table;

@group(0) @binding(0) var tex : texture_storage_2d<rgba8unorm, read_write>;

@fragment fn fs() {
    let tex = hasResource<texture_2d<i32>>(4u);
}
