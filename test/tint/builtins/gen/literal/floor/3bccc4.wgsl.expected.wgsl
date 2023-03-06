fn floor_3bccc4() {
  var res : vec4<f32> = floor(vec4<f32>(1.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_3bccc4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_3bccc4();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_3bccc4();
}
