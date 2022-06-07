fn mix_315264() {
  var arg_0 = vec3<f32>();
  var arg_1 = vec3<f32>();
  var arg_2 = 1.0;
  var res : vec3<f32> = mix(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_315264();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_315264();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_315264();
}
