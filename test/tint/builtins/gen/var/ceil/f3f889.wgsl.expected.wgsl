enable f16;

fn ceil_f3f889() {
  var arg_0 = f16();
  var res : f16 = ceil(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_f3f889();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_f3f889();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_f3f889();
}
