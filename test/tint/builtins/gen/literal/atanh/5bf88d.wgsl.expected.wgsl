enable f16;

fn atanh_5bf88d() {
  var res : vec2<f16> = atanh(vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_5bf88d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_5bf88d();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_5bf88d();
}
