@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgb10a2uint, read_write>;

fn textureLoad_19d8d6() -> vec4<u32> {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>(1i), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_19d8d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_19d8d6();
}
