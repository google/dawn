enable f16;

fn asin_3cfbd4() {
  var arg_0 = vec4<f16>(0.479248046875h);
  var res : vec4<f16> = asin(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_3cfbd4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_3cfbd4();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_3cfbd4();
}
