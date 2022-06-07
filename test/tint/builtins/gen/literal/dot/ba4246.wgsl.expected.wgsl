fn dot_ba4246() {
  var res : f32 = dot(vec3<f32>(), vec3<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_ba4246();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_ba4246();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_ba4246();
}
