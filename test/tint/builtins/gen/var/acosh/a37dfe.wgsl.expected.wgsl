enable f16;

fn acosh_a37dfe() {
  var arg_0 = 1.54296875h;
  var res : f16 = acosh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_a37dfe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_a37dfe();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_a37dfe();
}
