enable f16;

fn atanh_ec4b06() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = atanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_ec4b06();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_ec4b06();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_ec4b06();
}
