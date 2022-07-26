fn min_93cfc4() {
  var res : vec3<f32> = min(vec3<f32>(1.0f), vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_93cfc4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_93cfc4();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_93cfc4();
}
