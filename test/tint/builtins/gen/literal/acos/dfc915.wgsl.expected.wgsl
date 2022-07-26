fn acos_dfc915() {
  var res : vec2<f32> = acos(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_dfc915();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_dfc915();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_dfc915();
}
