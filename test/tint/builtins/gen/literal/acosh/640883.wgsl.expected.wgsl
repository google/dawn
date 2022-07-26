fn acosh_640883() {
  var res : vec2<f32> = acosh(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_640883();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_640883();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_640883();
}
