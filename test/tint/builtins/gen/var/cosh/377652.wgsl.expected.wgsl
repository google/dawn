fn cosh_377652() {
  var arg_0 = vec3<f32>(0.0f);
  var res : vec3<f32> = cosh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_377652();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_377652();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_377652();
}
