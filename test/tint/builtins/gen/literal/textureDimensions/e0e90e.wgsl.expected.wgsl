@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rg8snorm, write>;

fn textureDimensions_e0e90e() -> vec3<u32> {
  var res : vec3<u32> = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_e0e90e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_e0e90e();
}
