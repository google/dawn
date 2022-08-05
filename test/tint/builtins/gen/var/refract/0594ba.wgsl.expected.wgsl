enable f16;

fn refract_0594ba() {
  var arg_0 = vec4<f16>(f16());
  var arg_1 = vec4<f16>(f16());
  var arg_2 = f16();
  var res : vec4<f16> = refract(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_0594ba();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_0594ba();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_0594ba();
}
