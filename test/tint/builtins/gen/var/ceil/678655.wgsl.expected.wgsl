fn ceil_678655() {
  var arg_0 = 1.0f;
  var res : f32 = ceil(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_678655();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_678655();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_678655();
}
