enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8uint, read>;

fn textureLoad_50915c() {
  var res : vec4<u32> = textureLoad(arg_0, vec3<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_50915c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_50915c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_50915c();
}
