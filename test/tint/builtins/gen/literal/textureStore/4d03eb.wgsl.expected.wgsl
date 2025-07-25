requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r8snorm, read_write>;

fn textureStore_4d03eb() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_4d03eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_4d03eb();
}
