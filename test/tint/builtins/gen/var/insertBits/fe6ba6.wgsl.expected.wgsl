fn insertBits_fe6ba6() {
  var arg_0 = vec2<i32>(1i);
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec2<i32> = insertBits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_fe6ba6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_fe6ba6();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_fe6ba6();
}
