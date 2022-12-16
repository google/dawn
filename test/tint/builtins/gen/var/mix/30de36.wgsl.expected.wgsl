fn mix_30de36() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  const arg_2 = 1.0;
  var res = mix(arg_0, arg_1, arg_2);
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
