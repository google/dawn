fn floor_218952() {
  const arg_0 = vec4(1.5);
  var res = floor(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_218952();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_218952();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_218952();
}
