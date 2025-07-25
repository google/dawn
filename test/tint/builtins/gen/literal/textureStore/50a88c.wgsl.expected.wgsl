requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rg16uint, read_write>;

fn textureStore_50a88c() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_50a88c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_50a88c();
}
