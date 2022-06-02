fn length_becebf() {
  var arg_0 = vec4<f32>();
  var res : f32 = length(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_becebf();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  length_becebf();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  length_becebf();
}
