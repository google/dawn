fn countOneBits_af90e2() {
  var res : vec2<i32> = countOneBits(vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_af90e2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_af90e2();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_af90e2();
}
