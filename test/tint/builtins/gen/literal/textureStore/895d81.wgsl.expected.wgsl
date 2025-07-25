requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r8sint, read_write>;

fn textureStore_895d81() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_895d81();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_895d81();
}
