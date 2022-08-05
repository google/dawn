enable f16;

fn cos_5bc2c6() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = cos(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_5bc2c6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_5bc2c6();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_5bc2c6();
}
