fn exp2_8bd72d() {
  var res = exp2(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_8bd72d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_8bd72d();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_8bd72d();
}
