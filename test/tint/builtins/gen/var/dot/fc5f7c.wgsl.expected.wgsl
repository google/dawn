fn dot_fc5f7c() {
  var arg_0 = vec2<i32>(1i);
  var arg_1 = vec2<i32>(1i);
  var res : i32 = dot(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_fc5f7c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_fc5f7c();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_fc5f7c();
}
