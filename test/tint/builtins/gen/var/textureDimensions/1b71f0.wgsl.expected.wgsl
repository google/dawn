@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16sint, write>;

fn textureDimensions_1b71f0() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_1b71f0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_1b71f0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_1b71f0();
}
