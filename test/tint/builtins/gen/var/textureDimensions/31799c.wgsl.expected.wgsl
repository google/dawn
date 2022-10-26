@group(1) @binding(0) var arg_0 : texture_storage_3d<r32uint, write>;

fn textureDimensions_31799c() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_31799c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_31799c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_31799c();
}
