enable f16;

fn smoothstep_c43ebd() {
  var arg_0 = vec4<f16>(f16());
  var arg_1 = vec4<f16>(f16());
  var arg_2 = vec4<f16>(f16());
  var res : vec4<f16> = smoothstep(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_c43ebd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_c43ebd();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_c43ebd();
}
