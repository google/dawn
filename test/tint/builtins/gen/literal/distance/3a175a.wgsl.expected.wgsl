fn distance_3a175a() {
  var res = distance(vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_3a175a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_3a175a();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_3a175a();
}
