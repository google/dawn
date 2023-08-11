SKIP: FAILED


enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8snorm, read_write>;

fn textureStore_2c76db() {
  textureStore(arg_0, vec2<u32>(1u), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_2c76db();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_2c76db();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2c76db();
}

Failed to generate: builtins/gen/literal/textureStore/2c76db.wgsl:24:8 error: HLSL backend does not support extension 'chromium_experimental_read_write_storage_texture'
enable chromium_experimental_read_write_storage_texture;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

