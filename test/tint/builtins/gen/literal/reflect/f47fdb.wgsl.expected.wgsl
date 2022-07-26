fn reflect_f47fdb() {
  var res : vec3<f32> = reflect(vec3<f32>(1.0f), vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_f47fdb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_f47fdb();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_f47fdb();
}
