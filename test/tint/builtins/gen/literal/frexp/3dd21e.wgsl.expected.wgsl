enable f16;

fn frexp_3dd21e() {
  var res = frexp(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_3dd21e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_3dd21e();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_3dd21e();
}
