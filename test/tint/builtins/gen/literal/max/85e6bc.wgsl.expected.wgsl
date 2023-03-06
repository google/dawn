fn max_85e6bc() {
  var res : vec4<i32> = max(vec4<i32>(1i), vec4<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_85e6bc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_85e6bc();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_85e6bc();
}
