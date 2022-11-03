fn floor_218952() {
  var res = floor(vec4(1.5));
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
