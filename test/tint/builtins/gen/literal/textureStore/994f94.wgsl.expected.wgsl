@group(1) @binding(0) var arg_0 : texture_storage_1d<r16unorm, write>;

fn textureStore_994f94() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_994f94();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_994f94();
}
