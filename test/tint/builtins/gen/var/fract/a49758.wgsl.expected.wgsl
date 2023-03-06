fn fract_a49758() {
  var arg_0 = vec3<f32>(1.25f);
  var res : vec3<f32> = fract(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_a49758();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_a49758();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_a49758();
}
