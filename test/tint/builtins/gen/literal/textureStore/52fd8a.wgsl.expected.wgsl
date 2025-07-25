requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rg8unorm, read_write>;

fn textureStore_52fd8a() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_52fd8a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_52fd8a();
}
