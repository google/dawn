@group(1) @binding(0) var arg_0 : texture_3d<i32>;

fn textureDimensions_c871f3() {
  var res : vec3<u32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_c871f3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_c871f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_c871f3();
}
