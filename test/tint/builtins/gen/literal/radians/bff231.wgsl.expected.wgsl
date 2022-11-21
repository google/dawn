fn radians_bff231() {
  var res = radians(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_bff231();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_bff231();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_bff231();
}
