SKIP: FAILED


enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16sint, read_write>;

fn textureStore_86f713() {
  var arg_1 = vec3<i32>(1i);
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_86f713();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_86f713();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_86f713();
}

Failed to generate: builtins/gen/var/textureStore/86f713.wgsl:24:8 error: HLSL backend does not support extension 'chromium_experimental_read_write_storage_texture'
enable chromium_experimental_read_write_storage_texture;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

