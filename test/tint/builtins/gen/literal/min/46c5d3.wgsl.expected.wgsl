fn min_46c5d3() {
  var res : u32 = min(1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_46c5d3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_46c5d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_46c5d3();
}
