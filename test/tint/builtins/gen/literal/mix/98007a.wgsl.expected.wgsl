fn mix_98007a() {
  var res = mix(vec4(1.0), vec4(1.0), vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_98007a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_98007a();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_98007a();
}
