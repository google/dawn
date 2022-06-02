fn asin_7b6a44() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = asin(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_7b6a44();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  asin_7b6a44();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  asin_7b6a44();
}
