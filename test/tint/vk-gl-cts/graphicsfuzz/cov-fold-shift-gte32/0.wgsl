struct buf0 {
  one : u32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : u32;
  var b : u32;
  var c : u32;
  var d : u32;
  var e : u32;
  var f : u32;
  let x_41 : u32 = x_6.one;
  a = ((77u + x_41) >> 32u);
  let x_45 : u32 = x_6.one;
  b = ((3243u + x_45) >> 33u);
  let x_49 : u32 = x_6.one;
  c = ((23u + x_49) >> 345u);
  let x_53 : u32 = x_6.one;
  d = ((2395u + x_53) << 32u);
  let x_57 : u32 = x_6.one;
  e = ((290485u + x_57) << 33u);
  let x_61 : u32 = x_6.one;
  f = ((44321u + x_61) << 345u);
  let x_64 : u32 = a;
  if ((x_64 != 1u)) {
    a = 1u;
  }
  let x_68 : u32 = b;
  if ((x_68 != 0u)) {
    b = 0u;
  }
  let x_72 : u32 = c;
  if ((x_72 != 1u)) {
    c = 1u;
  }
  let x_76 : u32 = d;
  if ((x_76 != 0u)) {
    d = 0u;
  }
  let x_80 : u32 = e;
  if ((x_80 != 1u)) {
    e = 1u;
  }
  let x_84 : u32 = f;
  if ((x_84 != 0u)) {
    f = 0u;
  }
  let x_88 : u32 = a;
  let x_90 : u32 = b;
  let x_93 : u32 = c;
  let x_96 : u32 = d;
  let x_99 : u32 = e;
  let x_102 : u32 = f;
  if (((((((x_88 == 1u) & (x_90 == 0u)) & (x_93 == 1u)) & (x_96 == 0u)) & (x_99 == 1u)) & (x_102 == 0u))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
