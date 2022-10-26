@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16sint, write>;

fn textureDimensions_591981() {
  var res : vec3<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_591981();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_591981();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_591981();
}
