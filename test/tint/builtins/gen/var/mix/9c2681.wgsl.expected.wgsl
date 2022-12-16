fn mix_9c2681() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  const arg_2 = 1.0;
  var res = mix(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_9c2681();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_9c2681();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_9c2681();
}
