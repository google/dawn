fn step_334303() {
  var res : vec3<f32> = step(vec3<f32>(1.0f), vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_334303();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_334303();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_334303();
}
