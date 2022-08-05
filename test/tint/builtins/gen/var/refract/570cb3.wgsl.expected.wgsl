enable f16;

fn refract_570cb3() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var arg_2 = f16();
  var res : vec2<f16> = refract(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_570cb3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_570cb3();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_570cb3();
}
