fn tan_a0966f() {
  var res = tan(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_a0966f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_a0966f();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_a0966f();
}
