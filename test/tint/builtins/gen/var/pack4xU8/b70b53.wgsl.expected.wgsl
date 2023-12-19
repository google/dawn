fn pack4xU8_b70b53() {
  var arg_0 = vec4<u32>(1u);
  var res : u32 = pack4xU8(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pack4xU8_b70b53();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pack4xU8_b70b53();
}

@compute @workgroup_size(1)
fn compute_main() {
  pack4xU8_b70b53();
}
