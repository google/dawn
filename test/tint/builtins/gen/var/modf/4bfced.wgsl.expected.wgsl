fn modf_4bfced() {
  var arg_0 = vec4<f32>(-(1.5f));
  var res = modf(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_4bfced();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_4bfced();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_4bfced();
}
