[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  let x_51 : f32 = x_6.injectionSwitch.x;
  i = i32(x_51);
  let x_8 : i32 = i;
  switch(x_8) {
    case 0: {
      loop {
        let x_9 : i32 = i;
        i = (x_9 + 1);
        let x_11 : i32 = i;
        switch(x_11) {
          case 2: {
            let x_12 : i32 = i;
            i = (x_12 + 5);
          }
          case 1: {
            continue;
          }
          default: {
            let x_14 : i32 = i;
            i = (x_14 + 7);
          }
        }

        continuing {
          let x_16 : i32 = i;
          if ((x_16 > 200)) {
          } else {
            break;
          }
        }
      }
      let x_17 : i32 = i;
      if ((x_17 > 100)) {
        let x_18 : i32 = i;
        i = (x_18 - 2);
        break;
      }
      fallthrough;
    }
    default: {
      let x_20 : i32 = i;
      i = (x_20 - 3);
    }
  }
  let x_22 : i32 = i;
  if ((x_22 == -2)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
  }
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
