@group(0) @binding(0) var tex: texture_storage_2d<bgra8unorm, write>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  let value = vec4(1f, 2f, 3f, 4f);
  textureStore(tex, vec2(9, 8), value);
  return vec4<f32>();
}
