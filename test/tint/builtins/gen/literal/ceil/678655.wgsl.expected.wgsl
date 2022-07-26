fn ceil_678655() {
  var res : f32 = ceil(1.0f);
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
