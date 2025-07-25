requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rg32uint, read_write>;

fn textureStore_a58c2b() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_a58c2b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a58c2b();
}
