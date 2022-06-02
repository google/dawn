fn unpack2x16snorm_b4aea6() {
  var arg_0 = 1u;
  var res : vec2<f32> = unpack2x16snorm(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack2x16snorm_b4aea6();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  unpack2x16snorm_b4aea6();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  unpack2x16snorm_b4aea6();
}
