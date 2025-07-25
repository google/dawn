@group(1) @binding(0) var arg_0 : texture_storage_3d<r16sint, write>;

fn textureStore_2ac41d() {
  textureStore(arg_0, vec3<i32>(1i), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_2ac41d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2ac41d();
}
