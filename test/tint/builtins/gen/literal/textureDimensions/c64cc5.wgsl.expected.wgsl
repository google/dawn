@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16snorm, write>;

fn textureDimensions_c64cc5() -> vec3<u32> {
  var res : vec3<u32> = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_c64cc5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_c64cc5();
}
