fn pack4xI8Clamp_e42b2a() {
  var res : u32 = pack4xI8Clamp(vec4<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pack4xI8Clamp_e42b2a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pack4xI8Clamp_e42b2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  pack4xI8Clamp_e42b2a();
}
