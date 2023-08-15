enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32uint, read>;

fn textureLoad_22e963() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<u32>(1u), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_22e963();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_22e963();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_22e963();
}
