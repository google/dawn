@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba32sint, write>;

fn textureDimensions_83ee5a() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_83ee5a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_83ee5a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_83ee5a();
}
