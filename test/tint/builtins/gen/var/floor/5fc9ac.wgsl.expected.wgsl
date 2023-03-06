fn floor_5fc9ac() {
  var arg_0 = vec2<f32>(1.5f);
  var res : vec2<f32> = floor(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_5fc9ac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_5fc9ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_5fc9ac();
}
