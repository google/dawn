requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba32uint, read_write>;

fn textureStore_6e5c09() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_6e5c09();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6e5c09();
}
