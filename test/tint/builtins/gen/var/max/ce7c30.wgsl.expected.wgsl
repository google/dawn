fn max_ce7c30() {
  var arg_0 = 1i;
  var arg_1 = 1i;
  var res : i32 = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_ce7c30();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_ce7c30();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_ce7c30();
}
