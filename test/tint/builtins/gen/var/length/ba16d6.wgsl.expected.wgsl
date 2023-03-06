enable f16;

fn length_ba16d6() {
  var arg_0 = vec3<f16>(0.0h);
  var res : f16 = length(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_ba16d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_ba16d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_ba16d6();
}
