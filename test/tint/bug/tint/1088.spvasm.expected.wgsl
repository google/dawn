alias Arr = array<mat4x4f, 2u>;

struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr, 4u>;

struct LeftOver {
  /* @offset(0) */
  worldViewProjection : mat4x4f,
  /* @offset(64) */
  time : f32,
  /* @offset(80) */
  test2 : Arr,
  /* @offset(208) */
  test : Arr_1,
}

var<private> position_1 : vec3f;

@group(2) @binding(2) var<uniform> x_14 : LeftOver;

var<private> vUV : vec2f;

var<private> uv : vec2f;

var<private> normal : vec3f;

var<private> gl_Position : vec4f;

fn main_1() {
  var q : vec4f;
  var p : vec3f;
  let x_13 = position_1;
  q = vec4f(x_13.x, x_13.y, x_13.z, 1.0f);
  let x_21 = q;
  p = x_21.xyz;
  let x_27 = p.x;
  let x_41 = x_14.test[0i].el;
  let x_45 = position_1.y;
  let x_49 = x_14.time;
  p.x = (x_27 + sin(((x_41 * x_45) + x_49)));
  let x_55 = p.y;
  let x_57 = x_14.time;
  p.y = (x_55 + sin((x_57 + 4.0f)));
  let x_69 = x_14.worldViewProjection;
  let x_70 = p;
  gl_Position = (x_69 * vec4f(x_70.x, x_70.y, x_70.z, 1.0f));
  let x_83 = uv;
  vUV = x_83;
  let x_87 = gl_Position.y;
  gl_Position.y = (x_87 * -1.0f);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
  @location(0)
  vUV_1 : vec2f,
}

@vertex
fn main(@location(0) position_1_param : vec3f, @location(2) uv_param : vec2f, @location(1) normal_param : vec3f) -> main_out {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  return main_out(gl_Position, vUV);
}
