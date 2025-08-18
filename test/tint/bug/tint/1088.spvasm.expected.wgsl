struct tint_padded_array_element {
  @size(16u)
  tint_element : f32,
}

struct LeftOver_1_1 {
  worldViewProjection : mat4x4<f32>,
  time : f32,
  test2 : array<mat4x4<f32>, 2u>,
  test : array<tint_padded_array_element, 4u>,
}

@group(2u) @binding(2u) var<uniform> v : LeftOver_1_1;

struct gl_PerVertex {
  gl_Position : vec4<f32>,
  gl_PointSize : f32,
  gl_ClipDistance : array<f32, 1u>,
  gl_CullDistance : array<f32, 1u>,
}

var<private> v_1 : gl_PerVertex;

var<private> vUV : vec2<f32>;

fn main_inner(position : vec3<f32>, uv : vec2<f32>, normal : vec3<f32>) {
  var q : vec4<f32>;
  var p : vec3<f32>;
  q = vec4<f32>(position.x, position.y, position.z, 1.0f);
  p = q.xyz;
  p.x = (p.x + sin(((v.test[0i].tint_element * position.y) + v.time)));
  p.y = (p.y + sin((v.time + 4.0f)));
  let v_2 = v.worldViewProjection;
  let v_3 = p;
  v_1.gl_Position = (v_2 * vec4<f32>(v_3.x, v_3.y, v_3.z, 1.0f));
  vUV = uv;
  v_1.gl_Position.y = (v_1.gl_Position.y * -1.0f);
}

struct tint_symbol {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(0u)
  vUV : vec2<f32>,
}

@vertex
fn main(@location(0u) position : vec3<f32>, @location(2u) uv : vec2<f32>, @location(1u) normal : vec3<f32>) -> tint_symbol {
  main_inner(position, uv, normal);
  return tint_symbol(v_1.gl_Position, vUV);
}
