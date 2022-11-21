fn degrees_c0880c() {
  var res = degrees(vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_c0880c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_c0880c();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_c0880c();
}
