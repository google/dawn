requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgb10a2uint, read_write>;

fn textureStore_7f92c8() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_7f92c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_7f92c8();
}
