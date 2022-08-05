enable f16;

fn min_ac84d6() {
  var arg_0 = f16();
  var arg_1 = f16();
  var res : f16 = min(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_ac84d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_ac84d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_ac84d6();
}
