fn min_af326d() {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var res : f32 = min(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_af326d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_af326d();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_af326d();
}
