enable f16;

fn fma_bf21b6() {
  var res : vec2<f16> = fma(vec2<f16>(1.0h), vec2<f16>(1.0h), vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_bf21b6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_bf21b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_bf21b6();
}
