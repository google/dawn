enable f16;

fn cosh_2ed778() {
  var arg_0 = 0.0h;
  var res : f16 = cosh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_2ed778();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_2ed778();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_2ed778();
}
