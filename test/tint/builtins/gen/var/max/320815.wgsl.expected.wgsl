fn max_320815() {
  var arg_0 = vec2<u32>();
  var arg_1 = vec2<u32>();
  var res : vec2<u32> = max(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_320815();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  max_320815();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  max_320815();
}
