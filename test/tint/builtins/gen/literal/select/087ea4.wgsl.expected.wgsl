fn select_087ea4() {
  var res : vec4<u32> = select(vec4<u32>(1u), vec4<u32>(1u), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_087ea4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_087ea4();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_087ea4();
}
