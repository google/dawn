enable f16;

fn atan_a5f421() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = atan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_a5f421();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_a5f421();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_a5f421();
}
