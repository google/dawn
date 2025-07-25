requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba16unorm, read_write>;

fn textureStore_deef80() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_deef80();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_deef80();
}
