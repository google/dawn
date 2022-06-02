fn exp2_1f8680() {
  var arg_0 = vec3<f32>();
  var res : vec3<f32> = exp2(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_1f8680();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  exp2_1f8680();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  exp2_1f8680();
}
