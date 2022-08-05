enable f16;

fn exp2_b408e4() {
  var arg_0 = f16();
  var res : f16 = exp2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_b408e4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_b408e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_b408e4();
}
