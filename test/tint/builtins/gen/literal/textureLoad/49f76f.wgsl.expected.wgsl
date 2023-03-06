@group(1) @binding(0) var arg_0 : texture_multisampled_2d<u32>;

fn textureLoad_49f76f() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<u32>(1u), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_49f76f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_49f76f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_49f76f();
}
