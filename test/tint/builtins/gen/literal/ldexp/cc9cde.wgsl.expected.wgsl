fn ldexp_cc9cde() {
  var res : vec4<f32> = ldexp(vec4<f32>(1.0f), vec4<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_cc9cde();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_cc9cde();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_cc9cde();
}
