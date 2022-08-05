enable f16;

fn smoothstep_586e12() {
  var arg_0 = f16();
  var arg_1 = f16();
  var arg_2 = f16();
  var res : f16 = smoothstep(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_586e12();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_586e12();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_586e12();
}
