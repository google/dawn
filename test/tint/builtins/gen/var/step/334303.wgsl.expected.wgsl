fn step_334303() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var res : vec3<f32> = step(arg_0, arg_1);
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
