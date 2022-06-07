fn mix_2fadab() {
  var res : vec2<f32> = mix(vec2<f32>(), vec2<f32>(), 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_2fadab();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_2fadab();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_2fadab();
}
