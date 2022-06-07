@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8sint, write>;

fn textureDimensions_214dd4() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_214dd4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_214dd4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_214dd4();
}
