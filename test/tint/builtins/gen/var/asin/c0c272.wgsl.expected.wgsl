fn asin_c0c272() {
  var arg_0 = 1.0;
  var res : f32 = asin(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_c0c272();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  asin_c0c272();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  asin_c0c272();
}
