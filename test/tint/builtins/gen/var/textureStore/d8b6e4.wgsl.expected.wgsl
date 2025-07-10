requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba32uint, read_write>;

fn textureStore_d8b6e4() {
  var arg_1 = 1i;
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_d8b6e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d8b6e4();
}
