[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var value : i32;
  var y : f32;
  var x_31_phi : i32;
  i = 0;
  x_31_phi = 0;
  loop {
    let x_31 : i32 = x_31_phi;
    let x_37 : f32 = x_6.injectionSwitch.x;
    if ((x_31 < (2 + i32(x_37)))) {
    } else {
      break;
    }
    var x_55_phi : f32;
    var x_46_phi : f32;
    value = x_31;
    y = 0.5;
    x_55_phi = 0.5;
    x_46_phi = 0.5;
    switch(x_31) {
      case 0: {
        let x_54 : f32 = (0.5 + 0.5);
        y = x_54;
        x_55_phi = x_54;
        fallthrough;
      }
      case 1: {
        let x_55 : f32 = x_55_phi;
        let x_47 : f32 = clamp(1.0, 0.5, x_55);
        y = x_47;
        x_46_phi = x_47;
        fallthrough;
      }
      default: {
        fallthrough;
      }
      case 2: {
        let x_46 : f32 = x_46_phi;
        if ((x_46 == 1.0)) {
          x_GLF_color = vec4<f32>(f32((x_31 + 1)), 0.0, 0.0, 1.0);
          return;
        }
      }
    }

    continuing {
      let x_32 : i32 = (x_31 + 1);
      i = x_32;
      x_31_phi = x_32;
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
