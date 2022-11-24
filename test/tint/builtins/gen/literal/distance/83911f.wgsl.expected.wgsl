fn distance_83911f() {
  var res = distance(vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_83911f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_83911f();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_83911f();
}
