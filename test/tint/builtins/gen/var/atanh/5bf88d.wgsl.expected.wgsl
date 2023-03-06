enable f16;

fn atanh_5bf88d() {
  var arg_0 = vec2<f16>(0.5h);
  var res : vec2<f16> = atanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_5bf88d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_5bf88d();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_5bf88d();
}
