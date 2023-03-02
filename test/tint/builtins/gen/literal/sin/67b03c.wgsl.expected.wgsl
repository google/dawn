fn sin_67b03c() {
  var res = sin(vec3(1.57079632679000003037));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_67b03c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_67b03c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_67b03c();
}
