@group(1) @binding(0) var arg_0 : texture_3d<f32>;

fn textureLoad_019da0() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>(1i), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_019da0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_019da0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_019da0();
}
