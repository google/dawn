fn mix_42d11d() {
  var res = mix(vec2(1.0), vec2(1.0), 1.0);
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
