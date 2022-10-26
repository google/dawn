@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8uint, write>;

fn textureDimensions_c1dbf6() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_c1dbf6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_c1dbf6();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_c1dbf6();
}
