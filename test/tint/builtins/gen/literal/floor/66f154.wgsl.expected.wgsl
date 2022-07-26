fn floor_66f154() {
  var res : f32 = floor(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_66f154();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_66f154();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_66f154();
}
