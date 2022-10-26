@group(1) @binding(0) var arg_0 : texture_cube_array<i32>;

fn textureNumLayers_6b4321() {
  var res : u32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_6b4321();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_6b4321();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_6b4321();
}
