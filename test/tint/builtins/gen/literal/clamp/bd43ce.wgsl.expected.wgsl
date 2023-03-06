fn clamp_bd43ce() {
  var res : vec4<u32> = clamp(vec4<u32>(1u), vec4<u32>(1u), vec4<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_bd43ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_bd43ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_bd43ce();
}
