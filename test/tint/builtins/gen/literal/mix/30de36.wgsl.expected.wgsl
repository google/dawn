fn mix_30de36() {
  var res = mix(1.0, 1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_30de36();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_30de36();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_30de36();
}
