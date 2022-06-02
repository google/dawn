fn clamp_bd43ce() {
  var arg_0 = vec4<u32>();
  var arg_1 = vec4<u32>();
  var arg_2 = vec4<u32>();
  var res : vec4<u32> = clamp(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_bd43ce();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  clamp_bd43ce();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  clamp_bd43ce();
}
