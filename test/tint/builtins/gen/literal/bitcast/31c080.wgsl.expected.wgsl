fn bitcast_31c080() {
  var res : u32 = bitcast<u32>(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_31c080();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_31c080();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_31c080();
}
