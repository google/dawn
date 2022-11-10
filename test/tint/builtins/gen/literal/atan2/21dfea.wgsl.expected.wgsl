enable f16;

fn atan2_21dfea() {
  var res : vec3<f16> = atan2(vec3<f16>(1.0h), vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_21dfea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_21dfea();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_21dfea();
}
