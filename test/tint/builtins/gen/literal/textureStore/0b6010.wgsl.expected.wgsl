@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg16uint, write>;

fn textureStore_0b6010() {
  textureStore(arg_0, vec2<u32>(1u), 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_0b6010();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_0b6010();
}
