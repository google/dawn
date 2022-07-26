fn clamp_6c1749() {
  var res : vec2<i32> = clamp(vec2<i32>(1), vec2<i32>(1), vec2<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_6c1749();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_6c1749();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_6c1749();
}
