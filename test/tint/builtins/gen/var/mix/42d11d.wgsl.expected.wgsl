fn mix_42d11d() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  const arg_2 = 1.0;
  var res = mix(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_42d11d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_42d11d();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_42d11d();
}
