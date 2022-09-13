fn saturate_a5b571() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = saturate(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_a5b571();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_a5b571();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_a5b571();
}
