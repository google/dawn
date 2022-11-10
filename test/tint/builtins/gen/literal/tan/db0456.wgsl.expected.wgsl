enable f16;

fn tan_db0456() {
  var res : vec3<f16> = tan(vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_db0456();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_db0456();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_db0456();
}
