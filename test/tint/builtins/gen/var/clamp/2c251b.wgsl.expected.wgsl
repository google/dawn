enable f16;

fn clamp_2c251b() {
  var arg_0 = vec4<f16>(f16());
  var arg_1 = vec4<f16>(f16());
  var arg_2 = vec4<f16>(f16());
  var res : vec4<f16> = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_2c251b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_2c251b();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_2c251b();
}
