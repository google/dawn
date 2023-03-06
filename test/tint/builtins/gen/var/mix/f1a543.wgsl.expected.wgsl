enable f16;

fn mix_f1a543() {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = vec4<f16>(1.0h);
  var arg_2 = 1.0h;
  var res : vec4<f16> = mix(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_f1a543();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_f1a543();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_f1a543();
}
