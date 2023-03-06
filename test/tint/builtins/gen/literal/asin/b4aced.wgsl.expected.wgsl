enable f16;

fn asin_b4aced() {
  var res : vec2<f16> = asin(vec2<f16>(0.479248046875h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_b4aced();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_b4aced();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_b4aced();
}
