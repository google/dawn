enable f16;

fn cosh_43b672() {
  var res : vec2<f16> = cosh(vec2<f16>(0.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_43b672();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_43b672();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_43b672();
}
