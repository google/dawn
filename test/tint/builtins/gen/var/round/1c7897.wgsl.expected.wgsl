fn round_1c7897() {
  var arg_0 = vec3<f32>(3.5f);
  var res : vec3<f32> = round(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_1c7897();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_1c7897();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_1c7897();
}
