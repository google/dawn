[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
  let x_30 : f32 = x_6.injectionSwitch.x;
  switch(i32(x_30)) {
    case 0: {
      switch(1) {
        case 1: {
          let x_38 : f32 = gl_FragCoord.y;
          if ((x_38 >= 0.0)) {
            x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
            break;
          }
          discard;
        }
        default: {
        }
      }
      fallthrough;
    }
    case 42: {
    }
    default: {
    }
  }
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main([[builtin(position)]] gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
