fn cross_041cb0() {
  var res : vec3<f32> = cross(vec3<f32>(1.0f), vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cross_041cb0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cross_041cb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  cross_041cb0();
}
