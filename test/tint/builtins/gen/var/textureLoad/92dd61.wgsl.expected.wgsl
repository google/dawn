enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_1d<r8unorm, read_write>;

fn textureLoad_92dd61() {
  var arg_1 = 1u;
  var res : vec4<f32> = textureLoad(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_92dd61();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_92dd61();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_92dd61();
}
