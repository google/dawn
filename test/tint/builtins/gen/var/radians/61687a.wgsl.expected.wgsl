fn radians_61687a() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = radians(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_61687a();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  radians_61687a();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  radians_61687a();
}
