requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rg11b10ufloat, read_write>;

fn textureStore_6fd7ee() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_6fd7ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6fd7ee();
}
