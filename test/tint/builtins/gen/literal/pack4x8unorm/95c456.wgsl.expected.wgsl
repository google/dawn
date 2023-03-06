fn pack4x8unorm_95c456() {
  var res : u32 = pack4x8unorm(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pack4x8unorm_95c456();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pack4x8unorm_95c456();
}

@compute @workgroup_size(1)
fn compute_main() {
  pack4x8unorm_95c456();
}
