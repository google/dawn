requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba32sint, read_write>;

fn textureStore_c02743() {
  var arg_1 = 1i;
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_c02743();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c02743();
}
