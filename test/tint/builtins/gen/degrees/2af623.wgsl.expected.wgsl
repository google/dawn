fn degrees_2af623() {
  var res : vec3<f32> = degrees(vec3<f32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_2af623();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  degrees_2af623();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  degrees_2af623();
}
