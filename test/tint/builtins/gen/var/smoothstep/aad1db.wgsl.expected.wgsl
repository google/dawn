fn smoothstep_aad1db() {
  var arg_0 = vec3<f32>(2.0f);
  var arg_1 = vec3<f32>(4.0f);
  var arg_2 = vec3<f32>(3.0f);
  var res : vec3<f32> = smoothstep(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_aad1db();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_aad1db();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_aad1db();
}
