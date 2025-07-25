requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r16uint, read_write>;

fn textureStore_8bcb18() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_8bcb18();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8bcb18();
}
