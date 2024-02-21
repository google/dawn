fn bitcast_dce842() {
  var arg_0 = 1i;
  var res : u32 = bitcast<u32>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_dce842();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_dce842();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_dce842();
}
