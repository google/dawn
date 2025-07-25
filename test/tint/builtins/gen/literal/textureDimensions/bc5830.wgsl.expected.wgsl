@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d<rgb10a2unorm, write>;

fn textureDimensions_bc5830() -> vec2<u32> {
  var res : vec2<u32> = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_bc5830();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_bc5830();
}
