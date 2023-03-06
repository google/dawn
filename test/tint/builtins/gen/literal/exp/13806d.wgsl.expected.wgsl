enable f16;

fn exp_13806d() {
  var res : vec3<f16> = exp(vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_13806d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_13806d();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_13806d();
}
