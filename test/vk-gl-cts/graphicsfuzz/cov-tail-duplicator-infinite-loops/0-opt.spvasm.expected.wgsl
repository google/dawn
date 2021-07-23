type Arr = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr;
};

[[block]]
struct buf2 {
  zero : f32;
};

type Arr_1 = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr_1;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_5 : buf1;

[[group(0), binding(2)]] var<uniform> x_7 : buf2;

[[group(0), binding(0)]] var<uniform> x_10 : buf0;

fn main_1() {
  var i : i32;
  let x_38 : f32 = x_5.x_GLF_uniform_float_values[0];
  x_GLF_color = vec4<f32>(x_38, x_38, x_38, x_38);
  let x_41 : f32 = x_7.zero;
  let x_43 : f32 = x_5.x_GLF_uniform_float_values[0];
  if ((x_41 > x_43)) {
    loop {
      let x_53 : f32 = x_5.x_GLF_uniform_float_values[1];
      x_GLF_color = vec4<f32>(x_53, x_53, x_53, x_53);

      continuing {
        if (true) {
        } else {
          break;
        }
      }
    }
  } else {
    loop {
      loop {
        if (true) {
        } else {
          break;
        }
        let x_13 : i32 = x_10.x_GLF_uniform_int_values[1];
        i = x_13;
        loop {
          let x_14 : i32 = i;
          let x_15 : i32 = x_10.x_GLF_uniform_int_values[0];
          if ((x_14 < x_15)) {
          } else {
            break;
          }
          let x_73 : f32 = x_5.x_GLF_uniform_float_values[1];
          let x_75 : f32 = x_5.x_GLF_uniform_float_values[0];
          let x_77 : f32 = x_5.x_GLF_uniform_float_values[0];
          let x_79 : f32 = x_5.x_GLF_uniform_float_values[1];
          x_GLF_color = vec4<f32>(x_73, x_75, x_77, x_79);

          continuing {
            let x_16 : i32 = i;
            i = (x_16 + 1);
          }
        }
        break;
      }

      continuing {
        let x_82 : f32 = x_7.zero;
        let x_84 : f32 = x_5.x_GLF_uniform_float_values[0];
        if ((x_82 > x_84)) {
        } else {
          break;
        }
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
