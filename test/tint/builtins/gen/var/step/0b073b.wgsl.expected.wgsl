fn step_0b073b() {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var res : f32 = step(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_0b073b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_0b073b();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_0b073b();
}
