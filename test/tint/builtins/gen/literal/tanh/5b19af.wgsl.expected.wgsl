enable f16;

fn tanh_5b19af() {
  var res : f16 = tanh(1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_5b19af();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_5b19af();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_5b19af();
}
