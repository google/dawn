var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn nb_mod_f1_(limit : ptr<function, f32>) -> f32 {
  var x_injected_loop_counter : i32;
  var x_injected_loop_counter_1 : i32;
  let x_37 : f32 = *(limit);
  if ((1.0 >= x_37)) {
    return 1.0;
  }
  x_injected_loop_counter = 0;
  loop {
    let x_42 : bool = (0 < 2);
    x_injected_loop_counter_1 = 0;
    loop {
      let x_47 : bool = (0 < 1);
      return 1.0;
    }
  }
  return 0.0;
}

fn main_1() {
  var param : f32;
  let x_34 : f32 = gl_FragCoord.x;
  param = x_34;
  let x_35 : f32 = nb_mod_f1_(&(param));
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
