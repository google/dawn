@group(1) @binding(0) var arg_0 : texture_3d<u32>;

fn textureDimensions_6e6c7a() {
  var arg_1 = 1u;
  var res : vec3<u32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_6e6c7a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_6e6c7a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_6e6c7a();
}
