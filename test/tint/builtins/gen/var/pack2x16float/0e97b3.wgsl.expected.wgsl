fn pack2x16float_0e97b3() {
  var arg_0 = vec2<f32>(1.0f);
  var res : u32 = pack2x16float(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pack2x16float_0e97b3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pack2x16float_0e97b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  pack2x16float_0e97b3();
}
