fn radians_f96258() {
  var res : vec3<f32> = radians(vec3<f32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_f96258();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  radians_f96258();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  radians_f96258();
}
