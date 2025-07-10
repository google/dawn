requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<bgra8unorm, read_write>;

fn textureStore_bbd859() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_bbd859();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_bbd859();
}
