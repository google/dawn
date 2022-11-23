fn exp2_303753() {
  var res = exp2(vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_303753();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_303753();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_303753();
}
