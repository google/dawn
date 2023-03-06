enable f16;

fn atanh_e3b450() {
  var arg_0 = vec4<f16>(0.5h);
  var res : vec4<f16> = atanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_e3b450();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_e3b450();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_e3b450();
}
