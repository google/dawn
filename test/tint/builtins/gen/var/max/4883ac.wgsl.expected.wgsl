fn max_4883ac() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var res : vec3<f32> = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_4883ac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_4883ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_4883ac();
}
