fn round_1c7897() {
  var res : vec3<f32> = round(vec3<f32>(3.400000095f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_1c7897();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_1c7897();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_1c7897();
}
