enable f16;

fn select_10e73b() {
  var arg_0 = f16();
  var arg_1 = f16();
  var arg_2 = true;
  var res : f16 = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_10e73b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_10e73b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_10e73b();
}
