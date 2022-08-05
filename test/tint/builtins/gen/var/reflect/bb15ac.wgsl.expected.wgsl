enable f16;

fn reflect_bb15ac() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var res : vec2<f16> = reflect(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_bb15ac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_bb15ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_bb15ac();
}
