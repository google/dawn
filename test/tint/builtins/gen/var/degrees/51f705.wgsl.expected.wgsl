fn degrees_51f705() {
  var arg_0 = 1.0f;
  var res : f32 = degrees(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_51f705();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_51f705();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_51f705();
}
