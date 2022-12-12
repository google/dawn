fn mix_275cac() {
  var res = mix(vec4(1.0), vec4(1.0), 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_275cac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_275cac();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_275cac();
}
