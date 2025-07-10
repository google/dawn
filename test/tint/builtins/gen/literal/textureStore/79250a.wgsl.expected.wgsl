requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba16unorm, read_write>;

fn textureStore_79250a() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_79250a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_79250a();
}
