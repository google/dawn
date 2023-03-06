fn clamp_548fc7() {
  var arg_0 = vec3<u32>(1u);
  var arg_1 = vec3<u32>(1u);
  var arg_2 = vec3<u32>(1u);
  var res : vec3<u32> = clamp(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_548fc7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_548fc7();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_548fc7();
}
