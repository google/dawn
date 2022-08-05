enable f16;

fn clamp_235b29() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var arg_2 = vec2<f16>(f16());
  var res : vec2<f16> = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_235b29();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_235b29();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_235b29();
}
