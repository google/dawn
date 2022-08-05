enable f16;

fn exp_13806d() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = exp(arg_0);
}

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
