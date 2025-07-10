requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r16sint, read_write>;

fn textureStore_74eec8() {
  var arg_1 = 1i;
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_74eec8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_74eec8();
}
