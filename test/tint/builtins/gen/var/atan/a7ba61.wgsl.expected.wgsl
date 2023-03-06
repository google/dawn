enable f16;

fn atan_a7ba61() {
  var arg_0 = 1.0h;
  var res : f16 = atan(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_a7ba61();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_a7ba61();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_a7ba61();
}
