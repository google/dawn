fn select_c4a4ef() {
  var res : vec4<u32> = select(vec4<u32>(1u), vec4<u32>(1u), vec4<bool>(true));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_c4a4ef();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_c4a4ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_c4a4ef();
}
