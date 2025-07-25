requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgb10a2unorm, read_write>;

fn textureStore_5633f4() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_5633f4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5633f4();
}
