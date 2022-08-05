enable f16;

fn select_ed7c13() {
  var res : vec2<f16> = select(vec2<f16>(f16()), vec2<f16>(f16()), vec2<bool>(true));
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
