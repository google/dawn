@group(1) @binding(0) var arg_0 : texture_storage_3d<rgb10a2uint, write>;

fn textureStore_d1289f() {
  textureStore(arg_0, vec3<u32>(1u), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_d1289f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d1289f();
}
