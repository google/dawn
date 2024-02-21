fn bitcast_e6c18f() {
  var arg_0 = 1.0f;
  var res : u32 = bitcast<u32>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_e6c18f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_e6c18f();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_e6c18f();
}
