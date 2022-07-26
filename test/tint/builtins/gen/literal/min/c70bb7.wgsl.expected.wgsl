fn min_c70bb7() {
  var res : vec3<u32> = min(vec3<u32>(1u), vec3<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_c70bb7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_c70bb7();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_c70bb7();
}
