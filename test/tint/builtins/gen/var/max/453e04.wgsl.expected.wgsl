fn max_453e04() {
  var arg_0 = vec4<u32>(1u);
  var arg_1 = vec4<u32>(1u);
  var res : vec4<u32> = max(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_453e04();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_453e04();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_453e04();
}
