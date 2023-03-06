enable f16;

fn pow_4f33b2() {
  var res : vec4<f16> = pow(vec4<f16>(1.0h), vec4<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_4f33b2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_4f33b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_4f33b2();
}
