[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var data : array<f32, 2>;
  var x_32 : f32;
  x_32 = x_6.injectionSwitch.x;
  data[0] = x_32;
  let x_34 : ptr<function, f32> = &(data[1]);
  *(x_34) = x_32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  let x_35 : f32 = *(x_34);
  if ((x_35 > 1.0)) {
    var x_43_phi : f32;
    let x_39 : i32 = i32(x_32);
    x_43_phi = 0.0;
    switch(x_39) {
      case 0: {
        x_43_phi = 1.0;
        fallthrough;
      }
      case 1: {
        let x_43 : f32 = x_43_phi;
        data[x_39] = x_43;
        x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
      }
      default: {
      }
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
