requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r16float, read_write>;

fn textureStore_6a4045() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_6a4045();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6a4045();
}
