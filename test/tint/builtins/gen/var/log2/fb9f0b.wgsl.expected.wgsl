enable f16;

fn log2_fb9f0b() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = log2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_fb9f0b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_fb9f0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_fb9f0b();
}
