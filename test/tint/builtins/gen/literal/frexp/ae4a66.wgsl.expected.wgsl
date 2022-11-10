enable f16;

fn frexp_ae4a66() {
  var res = frexp(vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_ae4a66();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_ae4a66();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_ae4a66();
}
