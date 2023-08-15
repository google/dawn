enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16sint, read>;

fn textureDimensions_1b720f() {
  var res : vec3<u32> = textureDimensions(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_1b720f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_1b720f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_1b720f();
}
