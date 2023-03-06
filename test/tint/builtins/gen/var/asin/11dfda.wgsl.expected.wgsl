enable f16;

fn asin_11dfda() {
  var arg_0 = 0.479248046875h;
  var res : f16 = asin(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_11dfda();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_11dfda();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_11dfda();
}
