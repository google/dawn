enable f16;

fn select_ed7c13() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var arg_2 = vec2<bool>(true);
  var res : vec2<f16> = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_ed7c13();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_ed7c13();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_ed7c13();
}
