fn abs_7326de() {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = abs(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_7326de();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_7326de();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_7326de();
}
