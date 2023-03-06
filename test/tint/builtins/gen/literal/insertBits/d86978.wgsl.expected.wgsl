fn insertBits_d86978() {
  var res : vec4<i32> = insertBits(vec4<i32>(1i), vec4<i32>(1i), 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_d86978();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_d86978();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_d86978();
}
