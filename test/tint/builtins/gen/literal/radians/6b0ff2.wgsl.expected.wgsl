fn radians_6b0ff2() {
  var res : f32 = radians(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_6b0ff2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_6b0ff2();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_6b0ff2();
}
