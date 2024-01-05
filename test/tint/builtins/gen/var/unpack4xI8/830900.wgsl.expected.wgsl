fn unpack4xI8_830900() {
  var arg_0 = 1u;
  var res : vec4<i32> = unpack4xI8(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack4xI8_830900();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  unpack4xI8_830900();
}

@compute @workgroup_size(1)
fn compute_main() {
  unpack4xI8_830900();
}
