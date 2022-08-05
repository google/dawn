enable f16;

fn select_830dd9() {
  var res : vec4<f16> = select(vec4<f16>(f16()), vec4<f16>(f16()), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_830dd9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_830dd9();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_830dd9();
}
