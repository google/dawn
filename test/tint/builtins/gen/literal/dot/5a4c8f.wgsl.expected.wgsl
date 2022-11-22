fn dot_5a4c8f() {
  var res = dot(vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_5a4c8f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_5a4c8f();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_5a4c8f();
}
