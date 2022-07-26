fn acos_a610c4() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = acos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_a610c4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_a610c4();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_a610c4();
}
