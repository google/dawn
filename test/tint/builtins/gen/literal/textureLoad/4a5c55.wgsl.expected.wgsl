enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8sint, read_write>;

fn textureLoad_4a5c55() {
  var res : vec4<i32> = textureLoad(arg_0, vec3<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_4a5c55();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_4a5c55();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_4a5c55();
}
