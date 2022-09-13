enable f16;

fn saturate_dcde71() {
  var arg_0 = vec4<f16>(f16());
  var res : vec4<f16> = saturate(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_dcde71();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_dcde71();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_dcde71();
}
