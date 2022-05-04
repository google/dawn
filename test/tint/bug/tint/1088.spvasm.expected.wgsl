type Arr = array<mat4x4<f32>, 2u>;

struct strided_arr {
  @size(16)
  el : f32,
}

type Arr_1 = array<strided_arr, 4u>;

struct LeftOver {
  worldViewProjection : mat4x4<f32>,
  time : f32,
  @size(12)
  padding : u32,
  test2 : Arr,
  test : Arr_1,
}

var<private> position : vec3<f32>;

@group(2) @binding(2) var<uniform> x_14 : LeftOver;

var<private> vUV : vec2<f32>;

var<private> uv : vec2<f32>;

var<private> normal : vec3<f32>;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  var q : vec4<f32>;
  var p : vec3<f32>;
  let x_13 : vec3<f32> = position;
  q = vec4<f32>(x_13.x, x_13.y, x_13.z, 1.0);
  let x_21 : vec4<f32> = q;
  p = vec3<f32>(x_21.x, x_21.y, x_21.z);
  let x_27 : f32 = p.x;
  let x_41 : f32 = x_14.test[0i].el;
  let x_45 : f32 = position.y;
  let x_49 : f32 = x_14.time;
  p.x = (x_27 + sin(((x_41 * x_45) + x_49)));
  let x_55 : f32 = p.y;
  let x_57 : f32 = x_14.time;
  p.y = (x_55 + sin((x_57 + 4.0)));
  let x_69 : mat4x4<f32> = x_14.worldViewProjection;
  let x_70 : vec3<f32> = p;
  gl_Position = (x_69 * vec4<f32>(x_70.x, x_70.y, x_70.z, 1.0));
  let x_83 : vec2<f32> = uv;
  vUV = x_83;
  let x_87 : f32 = gl_Position.y;
  gl_Position.y = (x_87 * -1.0);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(0)
  vUV_1 : vec2<f32>,
}

@stage(vertex)
fn main(@location(0) position_param : vec3<f32>, @location(2) uv_param : vec2<f32>, @location(1) normal_param : vec3<f32>) -> main_out {
  position = position_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  return main_out(gl_Position, vUV);
}
