enable f16;

fn fma_e7abdc() {
  var arg_0 = vec3<f16>(f16());
  var arg_1 = vec3<f16>(f16());
  var arg_2 = vec3<f16>(f16());
  var res : vec3<f16> = fma(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_e7abdc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_e7abdc();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_e7abdc();
}
