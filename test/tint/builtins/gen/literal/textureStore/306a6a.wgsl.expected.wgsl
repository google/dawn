requires texel_buffers;

@group(1) @binding(0) var arg_0 : texel_buffer<rgba16float, read_write>;

fn textureStore_306a6a() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_306a6a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_306a6a();
}
