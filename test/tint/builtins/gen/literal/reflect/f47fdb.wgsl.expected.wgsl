fn reflect_f47fdb() {
  var res : vec3<f32> = reflect(vec3<f32>(1.0f), vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

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
