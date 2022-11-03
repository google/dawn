fn acos_4dac75() {
  var res = acos(vec4(0.87758256188999995));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_4dac75();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_4dac75();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_4dac75();
}
