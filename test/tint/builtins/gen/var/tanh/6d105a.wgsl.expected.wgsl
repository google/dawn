enable f16;

fn tanh_6d105a() {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = tanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_6d105a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_6d105a();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_6d105a();
}
