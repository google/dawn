requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba16snorm, read_write>;

fn textureStore_416b61() {
  var arg_1 = 1u;
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_416b61();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_416b61();
}
