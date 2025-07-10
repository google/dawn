requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba8sint, read_write>;

fn textureStore_2054bb() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_2054bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2054bb();
}
