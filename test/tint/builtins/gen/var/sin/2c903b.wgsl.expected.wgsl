enable f16;

fn sin_2c903b() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = sin(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_2c903b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_2c903b();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_2c903b();
}
