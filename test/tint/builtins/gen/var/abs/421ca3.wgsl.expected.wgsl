enable f16;

fn abs_421ca3() {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = abs(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_421ca3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_421ca3();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_421ca3();
}
