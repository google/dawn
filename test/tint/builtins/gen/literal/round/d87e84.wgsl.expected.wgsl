enable f16;

fn round_d87e84() {
  var res : vec2<f16> = round(vec2<f16>(3.5h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_d87e84();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_d87e84();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_d87e84();
}
