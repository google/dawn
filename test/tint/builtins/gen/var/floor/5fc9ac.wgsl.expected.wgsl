fn floor_5fc9ac() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = floor(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_5fc9ac();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  floor_5fc9ac();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  floor_5fc9ac();
}
