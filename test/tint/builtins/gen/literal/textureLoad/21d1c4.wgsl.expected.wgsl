@group(1) @binding(0) var arg_0 : texture_3d<f32>;

fn textureLoad_21d1c4() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<u32>(), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_21d1c4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_21d1c4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_21d1c4();
}
