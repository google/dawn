@group(0) @binding(0) var tex : texture_storage_2d<bgra8unorm, write>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  let value = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  textureStore(tex, vec2(9, 8), value);
  return vec4<f32>();
}
