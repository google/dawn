requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r8sint, read_write>;

fn textureStore_988438() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_988438();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_988438();
}
