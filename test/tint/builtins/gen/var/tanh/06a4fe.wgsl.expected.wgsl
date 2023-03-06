enable f16;

fn tanh_06a4fe() {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = tanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_06a4fe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_06a4fe();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_06a4fe();
}
