var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var data : array<f32, 1>;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  i = 0;
  loop {
    let x_6 : i32 = i;
    if ((x_6 < 1)) {
    } else {
      break;
    }
    let x_7 : i32 = i;
    let x_40 : f32 = data[x_7];
    let x_42 : f32 = data[0];
    if ((x_40 < x_42)) {
      if (false) {
        let x_8 : i32 = i;
        if ((f32(x_8) >= 1.0)) {
        }
      }
      switch(0) {
        case 1: {
          loop {
            if (true) {
            } else {
              break;
            }
          }
          loop {
          }
          fallthrough;
        }
        case 0: {
          data[0] = 2.0;
        }
        default: {
        }
      }
    }

    continuing {
      let x_9 : i32 = i;
      i = (x_9 + 1);
    }
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
