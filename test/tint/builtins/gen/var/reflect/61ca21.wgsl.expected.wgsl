enable f16;

fn reflect_61ca21() {
  var arg_0 = vec3<f16>(f16());
  var arg_1 = vec3<f16>(f16());
  var res : vec3<f16> = reflect(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_61ca21();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_61ca21();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_61ca21();
}
