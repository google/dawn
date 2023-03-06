@group(1) @binding(0) var arg_0 : texture_2d_array<u32>;

fn textureLoad_53e142() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<u32>(1u), 1i, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_53e142();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_53e142();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_53e142();
}
