@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rg16sint, read_write>;

fn textureLoad_39e596() -> vec4<i32> {
  var res : vec4<i32> = textureLoad(arg_0, vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_39e596();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_39e596();
}
