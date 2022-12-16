fn mix_343c49() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  const arg_2 = vec3(1.0);
  var res = mix(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_343c49();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_343c49();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_343c49();
}
