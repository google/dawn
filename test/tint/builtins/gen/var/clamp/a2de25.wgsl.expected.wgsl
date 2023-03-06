fn clamp_a2de25() {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : u32 = clamp(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_a2de25();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_a2de25();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_a2de25();
}
