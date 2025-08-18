struct gl_PerVertex {
  gl_Position : vec4<f32>,
  gl_PointSize : f32,
  gl_ClipDistance : array<f32, 1u>,
  gl_CullDistance : array<f32, 1u>,
}

var<private> v : gl_PerVertex;

fn main_inner() {
  v.gl_Position = vec4<f32>();
}

@vertex
fn main() -> @builtin(position) vec4<f32> {
  main_inner();
  return v.gl_Position;
}
