fn modf_9b75f7() {
  var arg_0 = vec3<f32>();
  var res = modf(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_9b75f7();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  modf_9b75f7();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  modf_9b75f7();
}
