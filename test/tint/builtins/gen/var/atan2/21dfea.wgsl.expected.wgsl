enable f16;

fn atan2_21dfea() {
  var arg_0 = vec3<f16>(f16());
  var arg_1 = vec3<f16>(f16());
  var res : vec3<f16> = atan2(arg_0, arg_1);
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
