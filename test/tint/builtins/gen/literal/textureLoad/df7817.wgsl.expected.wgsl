@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r8uint, read_write>;

fn textureLoad_df7817() -> vec4<u32> {
  var res : vec4<u32> = textureLoad(arg_0, vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_df7817();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_df7817();
}
