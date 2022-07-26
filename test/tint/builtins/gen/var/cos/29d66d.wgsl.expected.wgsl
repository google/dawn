fn cos_29d66d() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = cos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_29d66d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_29d66d();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_29d66d();
}
