enable f16;

fn fma_bf21b6() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var arg_2 = vec2<f16>(f16());
  var res : vec2<f16> = fma(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_bf21b6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_bf21b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_bf21b6();
}
