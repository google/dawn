enable f16;

fn mix_f1a543() {
  var arg_0 = vec4<f16>(f16());
  var arg_1 = vec4<f16>(f16());
  var arg_2 = f16();
  var res : vec4<f16> = mix(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_f1a543();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_f1a543();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_f1a543();
}
