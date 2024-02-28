enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_1d<r8unorm, read>;

fn textureDimensions_fdbae8() {
  var res : u32 = textureDimensions(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_fdbae8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_fdbae8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_fdbae8();
}
