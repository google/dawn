requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r8unorm, read_write>;

fn textureStore_5a1c4f() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_5a1c4f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5a1c4f();
}
