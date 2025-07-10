requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rg16uint, read_write>;

fn textureStore_394c03() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_394c03();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_394c03();
}
