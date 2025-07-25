requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rg8sint, read_write>;

fn textureStore_6ab209() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_6ab209();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6ab209();
}
