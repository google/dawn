fn countOneBits_65d2ae() {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = countOneBits(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_65d2ae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_65d2ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_65d2ae();
}
