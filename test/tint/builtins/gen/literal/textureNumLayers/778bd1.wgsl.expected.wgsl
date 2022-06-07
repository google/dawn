@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

fn textureNumLayers_778bd1() {
  var res : i32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_778bd1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_778bd1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_778bd1();
}
