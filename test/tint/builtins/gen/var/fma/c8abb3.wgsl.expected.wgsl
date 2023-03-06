enable f16;

fn fma_c8abb3() {
  var arg_0 = 1.0h;
  var arg_1 = 1.0h;
  var arg_2 = 1.0h;
  var res : f16 = fma(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_c8abb3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_c8abb3();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_c8abb3();
}
