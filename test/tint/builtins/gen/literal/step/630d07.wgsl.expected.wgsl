enable f16;

fn step_630d07() {
  var res : f16 = step(1.0h, 1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_630d07();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_630d07();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_630d07();
}
