enable f16;

fn smoothstep_586e12() {
  var arg_0 = 2.0h;
  var arg_1 = 4.0h;
  var arg_2 = 3.0h;
  var res : f16 = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

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
