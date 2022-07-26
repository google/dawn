fn radians_f96258() {
  var res : vec3<f32> = radians(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_f96258();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_f96258();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_f96258();
}
