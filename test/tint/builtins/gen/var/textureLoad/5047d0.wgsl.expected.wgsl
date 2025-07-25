@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rg8uint, read_write>;

fn textureLoad_5047d0() -> vec4<u32> {
  var arg_1 = 1i;
  var res : vec4<u32> = textureLoad(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_5047d0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_5047d0();
}
