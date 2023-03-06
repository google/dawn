enable f16;

fn acosh_5f49d8() {
  var res : vec2<f16> = acosh(vec2<f16>(1.54296875h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_5f49d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_5f49d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_5f49d8();
}
