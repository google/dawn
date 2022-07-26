fn max_a93419() {
  var res : vec4<f32> = max(vec4<f32>(1.0f), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_a93419();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_a93419();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_a93419();
}
