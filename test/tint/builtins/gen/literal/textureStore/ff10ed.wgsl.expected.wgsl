@group(1) @binding(0) var arg_0 : texture_storage_3d<r16sint, write>;

fn textureStore_ff10ed() {
  textureStore(arg_0, vec3<u32>(1u), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_ff10ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_ff10ed();
}
