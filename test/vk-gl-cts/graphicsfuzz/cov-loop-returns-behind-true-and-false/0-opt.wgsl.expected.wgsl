type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

var<private> x_GLF_global_loop_count : i32;

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  x_GLF_global_loop_count = 0;
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[0];
  switch(x_27) {
    case 0: {
      if (true) {
        let x_34 : i32 = x_6.x_GLF_uniform_int_values[1];
        let x_35 : f32 = f32(x_34);
        x_GLF_color = vec4<f32>(x_35, x_35, x_35, x_35);
        return;
      }
      fallthrough;
    }
    case 1: {
      if (true) {
        let x_40 : i32 = x_6.x_GLF_uniform_int_values[0];
        let x_43 : i32 = x_6.x_GLF_uniform_int_values[1];
        let x_46 : i32 = x_6.x_GLF_uniform_int_values[1];
        let x_49 : i32 = x_6.x_GLF_uniform_int_values[0];
        x_GLF_color = vec4<f32>(f32(x_40), f32(x_43), f32(x_46), f32(x_49));
        if (false) {
          let x_55 : i32 = x_6.x_GLF_uniform_int_values[0];
          let x_56 : f32 = f32(x_55);
          x_GLF_color = vec4<f32>(x_56, x_56, x_56, x_56);
          loop {
            let x_62 : i32 = x_GLF_global_loop_count;
            x_GLF_global_loop_count = (x_62 + 1);
            if (false) {
              return;
            }
            if (true) {
              return;
            }

            continuing {
              let x_68 : i32 = x_GLF_global_loop_count;
              if ((x_68 < 100)) {
              } else {
                break;
              }
            }
          }
        }
      }
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
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
