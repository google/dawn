fn modf_180fed() {
  var arg_0 = 1.0;
  var res = modf(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_180fed();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  modf_180fed();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  modf_180fed();
}
