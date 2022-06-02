fn mix_2fadab() {
  var arg_0 = vec2<f32>();
  var arg_1 = vec2<f32>();
  var arg_2 = 1.0;
  var res : vec2<f32> = mix(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_2fadab();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  mix_2fadab();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  mix_2fadab();
}
