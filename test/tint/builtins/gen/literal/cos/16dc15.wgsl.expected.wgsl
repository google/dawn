fn cos_16dc15() {
  var res : vec3<f32> = cos(vec3<f32>(1.0f));
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
