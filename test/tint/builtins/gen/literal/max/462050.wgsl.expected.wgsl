fn max_462050() {
  var res : vec2<f32> = max(vec2<f32>(1.0f), vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_462050();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_462050();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_462050();
}
