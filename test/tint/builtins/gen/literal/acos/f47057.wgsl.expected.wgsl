enable f16;

fn acos_f47057() {
  var res : vec3<f16> = acos(vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_f47057();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_f47057();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_f47057();
}
