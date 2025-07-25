@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d<rg16float, read_write>;

fn textureDimensions_c2c332() -> vec2<u32> {
  var res : vec2<u32> = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_c2c332();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_c2c332();
}
