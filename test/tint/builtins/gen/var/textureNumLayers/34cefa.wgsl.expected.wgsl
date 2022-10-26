@group(1) @binding(0) var arg_0 : texture_cube_array<f32>;

fn textureNumLayers_34cefa() {
  var res : u32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_34cefa();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_34cefa();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_34cefa();
}
