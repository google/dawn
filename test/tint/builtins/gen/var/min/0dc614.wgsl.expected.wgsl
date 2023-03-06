fn min_0dc614() {
  var arg_0 = vec4<u32>(1u);
  var arg_1 = vec4<u32>(1u);
  var res : vec4<u32> = min(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_0dc614();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_0dc614();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_0dc614();
}
