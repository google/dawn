requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba8snorm, read_write>;

fn textureStore_8af629() {
  var arg_1 = 1u;
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_8af629();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8af629();
}
