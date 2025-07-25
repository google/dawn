requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba16sint, read_write>;

fn textureStore_2bf55a() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_2bf55a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2bf55a();
}
