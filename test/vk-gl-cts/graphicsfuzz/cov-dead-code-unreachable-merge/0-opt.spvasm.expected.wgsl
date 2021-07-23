[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

var<private> gl_FragCoord : vec4<f32>;

var<private> array0 : array<f32, 3>;

var<private> array1 : array<f32, 3>;

[[group(0), binding(0)]] var<uniform> x_11 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var q : i32;
  var i : i32;
  var c : i32;
  q = 0;
  let x_55 : f32 = gl_FragCoord.x;
  i = (i32(x_55) % 3);
  c = 0;
  loop {
    let x_14 : i32 = c;
    if ((x_14 < 3)) {
    } else {
      break;
    }
    let x_15 : i32 = c;
    array0[x_15] = 0.0;
    let x_16 : i32 = c;
    array1[x_16] = 0.0;
    let x_65 : f32 = x_11.injectionSwitch.x;
    let x_18 : i32 = q;
    switch((i32(x_65) + x_18)) {
      case 51: {
        loop {
          if (true) {
          } else {
            break;
          }
        }
        let x_20 : i32 = c;
        array0[x_20] = 1.0;
        fallthrough;
      }
      case 61: {
        array1[0] = 1.0;
        let x_21 : i32 = c;
        array1[x_21] = 1.0;
      }
      case 0: {
        q = 61;
      }
      default: {
      }
    }

    continuing {
      let x_22 : i32 = c;
      c = (x_22 + 1);
    }
  }
  let x_24 : i32 = i;
  let x_79 : f32 = array1[x_24];
  let x_25 : i32 = i;
  let x_81 : f32 = array0[x_25];
  let x_26 : i32 = i;
  let x_83 : f32 = array0[x_26];
  x_GLF_color = vec4<f32>(x_79, x_81, x_83, 1.0);
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
