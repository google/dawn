fn floor_60d7ea() {
  var res : vec3<f32> = floor(vec3<f32>(1.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_60d7ea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_60d7ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_60d7ea();
}
