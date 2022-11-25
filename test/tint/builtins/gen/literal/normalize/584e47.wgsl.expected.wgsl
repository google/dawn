fn normalize_584e47() {
  var res = normalize(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_584e47();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_584e47();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_584e47();
}
