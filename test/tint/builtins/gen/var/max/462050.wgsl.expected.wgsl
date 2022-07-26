fn max_462050() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var res : vec2<f32> = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_462050();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_462050();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_462050();
}
