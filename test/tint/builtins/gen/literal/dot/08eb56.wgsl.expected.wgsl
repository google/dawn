fn dot_08eb56() {
  var res = dot(vec4(1.0), vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_08eb56();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_08eb56();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_08eb56();
}
