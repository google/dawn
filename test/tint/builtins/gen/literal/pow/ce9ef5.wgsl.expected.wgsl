enable f16;

fn pow_ce9ef5() {
  var res : f16 = pow(1.0h, 1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_ce9ef5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_ce9ef5();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_ce9ef5();
}
