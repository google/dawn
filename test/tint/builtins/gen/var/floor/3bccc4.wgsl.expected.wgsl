fn floor_3bccc4() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = floor(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_3bccc4();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  floor_3bccc4();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  floor_3bccc4();
}
