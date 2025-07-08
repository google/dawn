@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8snorm, write>;

fn textureStore_c1984f() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_c1984f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c1984f();
}
