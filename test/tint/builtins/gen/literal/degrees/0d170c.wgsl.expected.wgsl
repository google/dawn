fn degrees_0d170c() {
  var res : vec4<f32> = degrees(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_0d170c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_0d170c();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_0d170c();
}
