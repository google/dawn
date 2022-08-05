enable f16;

fn sin_66a59f() {
  var arg_0 = f16();
  var res : f16 = sin(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_66a59f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_66a59f();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_66a59f();
}
