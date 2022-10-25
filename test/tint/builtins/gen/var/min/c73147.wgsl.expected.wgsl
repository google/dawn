fn min_c73147() {
  var arg_0 = 1i;
  var arg_1 = 1i;
  var res : i32 = min(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_c73147();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_c73147();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_c73147();
}
