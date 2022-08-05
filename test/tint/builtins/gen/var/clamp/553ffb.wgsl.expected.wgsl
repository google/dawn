enable f16;

fn clamp_553ffb() {
  var arg_0 = f16();
  var arg_1 = f16();
  var arg_2 = f16();
  var res : f16 = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_553ffb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_553ffb();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_553ffb();
}
