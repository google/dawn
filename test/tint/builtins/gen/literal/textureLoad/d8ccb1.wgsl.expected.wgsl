requires texel_buffers;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba16uint, read_write>;

fn textureLoad_d8ccb1() -> vec4<u32> {
  var res : vec4<u32> = textureLoad(arg_0, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_d8ccb1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_d8ccb1();
}
