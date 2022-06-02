fn unpack4x8snorm_523fb3() {
  var arg_0 = 1u;
  var res : vec4<f32> = unpack4x8snorm(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack4x8snorm_523fb3();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  unpack4x8snorm_523fb3();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  unpack4x8snorm_523fb3();
}
