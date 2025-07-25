requires texel_buffers;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texel_buffer<rg8sint, read_write>;

fn textureLoad_3cfeac() -> vec4<i32> {
  var res : vec4<i32> = textureLoad(arg_0, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_3cfeac();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_3cfeac();
}
