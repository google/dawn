requires texel_buffers;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texel_buffer<rg11b10ufloat, read_write>;

fn textureDimensions_e1ef3d() -> u32 {
  var res : u32 = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_e1ef3d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_e1ef3d();
}
