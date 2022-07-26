fn acos_8e2acf() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = acos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_8e2acf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_8e2acf();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_8e2acf();
}
