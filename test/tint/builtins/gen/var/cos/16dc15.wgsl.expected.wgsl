fn cos_16dc15() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = cos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_16dc15();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_16dc15();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_16dc15();
}
