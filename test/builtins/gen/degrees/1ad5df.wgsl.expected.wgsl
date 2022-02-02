fn degrees_1ad5df() {
  var res : vec2<f32> = degrees(vec2<f32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_1ad5df();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  degrees_1ad5df();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  degrees_1ad5df();
}
