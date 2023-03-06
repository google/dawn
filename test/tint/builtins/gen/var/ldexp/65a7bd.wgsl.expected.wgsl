fn ldexp_65a7bd() {
  var arg_0 = vec4<f32>(1.0f);
  const arg_1 = vec4(1);
  var res : vec4<f32> = ldexp(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_65a7bd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_65a7bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_65a7bd();
}
