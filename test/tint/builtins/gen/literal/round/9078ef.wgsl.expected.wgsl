enable f16;

fn round_9078ef() {
  var res : f16 = round(3.5h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_9078ef();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_9078ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_9078ef();
}
