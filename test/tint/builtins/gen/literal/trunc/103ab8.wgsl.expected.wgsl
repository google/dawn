enable f16;

fn trunc_103ab8() {
  var res : vec3<f16> = trunc(vec3<f16>(1.5h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_103ab8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_103ab8();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_103ab8();
}
