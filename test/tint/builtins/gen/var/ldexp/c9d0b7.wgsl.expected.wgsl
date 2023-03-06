fn ldexp_c9d0b7() {
  var arg_0 = 1.0f;
  const arg_1 = 1;
  var res : f32 = ldexp(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_c9d0b7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_c9d0b7();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_c9d0b7();
}
