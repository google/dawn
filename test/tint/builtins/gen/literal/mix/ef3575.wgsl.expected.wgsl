fn mix_ef3575() {
  var res = mix(vec2(1.0), vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_ef3575();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_ef3575();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_ef3575();
}
