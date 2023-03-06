enable f16;

fn cos_5bc2c6() {
  var arg_0 = vec2<f16>(0.0h);
  var res : vec2<f16> = cos(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_5bc2c6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_5bc2c6();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_5bc2c6();
}
