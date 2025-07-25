@group(1) @binding(0) var arg_0 : texture_storage_3d<rg16uint, write>;

fn textureStore_bd62a3() {
  textureStore(arg_0, vec3<i32>(1i), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_bd62a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_bd62a3();
}
