enable f16;

fn ldexp_624e0c() {
  var res : f16 = ldexp(1.0h, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_624e0c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_624e0c();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_624e0c();
}
