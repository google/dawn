requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<r32sint, read_write>;

fn textureStore_09f60f() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_09f60f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_09f60f();
}
