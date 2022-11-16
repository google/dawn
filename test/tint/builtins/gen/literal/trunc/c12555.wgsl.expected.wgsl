fn trunc_c12555() {
  var res = trunc(vec2(1.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_c12555();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_c12555();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_c12555();
}
