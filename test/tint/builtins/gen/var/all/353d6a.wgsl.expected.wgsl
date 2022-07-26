fn all_353d6a() {
  var arg_0 = true;
  var res : bool = all(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  all_353d6a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  all_353d6a();
}

@compute @workgroup_size(1)
fn compute_main() {
  all_353d6a();
}
