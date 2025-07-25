requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<bgra8unorm, read_write>;

fn textureStore_bbd859() {
  var arg_1 = 1i;
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_bbd859();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_bbd859();
}
