enable f16;

fn fma_e7abdc() {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = vec3<f16>(1.0h);
  var arg_2 = vec3<f16>(1.0h);
  var res : vec3<f16> = fma(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_e7abdc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_e7abdc();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_e7abdc();
}
