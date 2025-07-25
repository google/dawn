@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d<rg16sint, read_write>;

fn textureLoad_e14fdf() -> vec4<i32> {
  var res : vec4<i32> = textureLoad(arg_0, vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_e14fdf();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_e14fdf();
}
