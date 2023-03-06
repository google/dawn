enable f16;

fn atan_1e1764() {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = atan(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_1e1764();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_1e1764();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_1e1764();
}
