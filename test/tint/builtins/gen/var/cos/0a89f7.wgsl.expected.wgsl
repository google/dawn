enable f16;

fn cos_0a89f7() {
  var arg_0 = vec4<f16>(0.0h);
  var res : vec4<f16> = cos(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_0a89f7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_0a89f7();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_0a89f7();
}
