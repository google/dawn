@group(1) @binding(0) var arg_0 : texture_3d<u32>;

fn textureLoad_1c562a() {
  var res : vec4<u32> = textureLoad(arg_0, vec3<u32>(1u), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_1c562a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_1c562a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_1c562a();
}
