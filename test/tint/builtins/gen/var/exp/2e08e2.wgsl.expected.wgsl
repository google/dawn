enable f16;

fn exp_2e08e2() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = exp(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_2e08e2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_2e08e2();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_2e08e2();
}
