@group(0) @binding(0) var tex : texture_2d<f32>;

@group(1) @binding(0) var store : texture_storage_2d<r32float, read_write>;

@fragment
fn main() {
  let a = tex;
  let b = a;
  let c = b;
  var res : vec4f = textureLoad(c, vec2i(1i), 0);
  textureStore(store, vec2i(0i), res);
}
