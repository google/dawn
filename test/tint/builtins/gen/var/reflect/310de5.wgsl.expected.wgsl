enable f16;

fn reflect_310de5() {
  var arg_0 = vec4<f16>(f16());
  var arg_1 = vec4<f16>(f16());
  var res : vec4<f16> = reflect(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_310de5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_310de5();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_310de5();
}
