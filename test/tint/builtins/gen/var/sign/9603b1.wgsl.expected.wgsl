fn sign_9603b1() {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = sign(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_9603b1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_9603b1();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_9603b1();
}
