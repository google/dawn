enable f16;

fn smoothstep_c43ebd() {
  var arg_0 = vec4<f16>(2.0h);
  var arg_1 = vec4<f16>(4.0h);
  var arg_2 = vec4<f16>(3.0h);
  var res : vec4<f16> = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

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
