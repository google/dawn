enable f16;

fn cos_0a89f7() {
  var arg_0 = vec4<f16>(f16());
  var res : vec4<f16> = cos(arg_0);
}

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
