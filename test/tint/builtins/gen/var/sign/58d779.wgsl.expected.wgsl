fn sign_58d779() {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = sign(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_58d779();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_58d779();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_58d779();
}
