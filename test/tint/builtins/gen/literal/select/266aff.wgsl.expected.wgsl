fn select_266aff() {
  var res : vec2<f32> = select(vec2<f32>(), vec2<f32>(), vec2<bool>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_266aff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_266aff();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_266aff();
}
