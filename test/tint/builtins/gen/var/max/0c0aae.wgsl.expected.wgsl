fn max_0c0aae() {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var res : u32 = max(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_0c0aae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_0c0aae();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_0c0aae();
}
