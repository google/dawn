fn select_266aff() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var arg_2 = vec2<bool>(true);
  var res : vec2<f32> = select(arg_0, arg_1, arg_2);
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
