fn mix_0c8c33() {
  var arg_0 = vec3<f32>();
  var arg_1 = vec3<f32>();
  var arg_2 = vec3<f32>();
  var res : vec3<f32> = mix(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_0c8c33();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  mix_0c8c33();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  mix_0c8c33();
}
