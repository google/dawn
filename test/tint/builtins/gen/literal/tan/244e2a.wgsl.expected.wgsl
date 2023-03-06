fn tan_244e2a() {
  var res : vec4<f32> = tan(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_244e2a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_244e2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_244e2a();
}
