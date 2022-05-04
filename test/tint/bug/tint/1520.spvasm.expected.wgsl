struct UniformBuffer {
  @size(16)
  padding : u32,
  unknownInput_S1_c0 : f32,
  @size(12)
  padding_1 : u32,
  ucolorRed_S1_c0 : vec4<f32>,
  ucolorGreen_S1_c0 : vec4<f32>,
  umatrix_S1 : mat3x3<f32>,
}

@binding(0) @group(0) var<uniform> x_4 : UniformBuffer;

var<private> sk_FragColor : vec4<f32>;

var<private> sk_Clockwise : bool;

var<private> vcolor_S0 : vec4<f32>;

fn test_int_S1_c0_b() -> bool {
  var unknown : i32;
  var ok : bool;
  var val : vec4<i32>;
  var x_40 : bool;
  var x_54 : bool;
  var x_65 : bool;
  var x_41_phi : bool;
  var x_55_phi : bool;
  var x_66_phi : bool;
  let x_26 : f32 = x_4.unknownInput_S1_c0;
  let x_27 : i32 = i32(x_26);
  unknown = x_27;
  ok = true;
  x_41_phi = false;
  if (true) {
    x_40 = all(((vec4<i32>(0i, 0i, 0i, 0i) / vec4<i32>(x_27, x_27, x_27, x_27)) == vec4<i32>(0i, 0i, 0i, 0i)));
    x_41_phi = x_40;
  }
  let x_41 : bool = x_41_phi;
  ok = x_41;
  let x_44 : vec4<i32> = vec4<i32>(x_27, x_27, x_27, x_27);
  val = x_44;
  let x_47 : vec4<i32> = (x_44 + vec4<i32>(1i, 1i, 1i, 1i));
  val = x_47;
  let x_48 : vec4<i32> = (x_47 - vec4<i32>(1i, 1i, 1i, 1i));
  val = x_48;
  let x_49 : vec4<i32> = (x_48 + vec4<i32>(1i, 1i, 1i, 1i));
  val = x_49;
  let x_50 : vec4<i32> = (x_49 - vec4<i32>(1i, 1i, 1i, 1i));
  val = x_50;
  x_55_phi = false;
  if (x_41) {
    x_54 = all((x_50 == x_44));
    x_55_phi = x_54;
  }
  let x_55 : bool = x_55_phi;
  ok = x_55;
  let x_58 : vec4<i32> = (x_50 * vec4<i32>(2i, 2i, 2i, 2i));
  val = x_58;
  let x_59 : vec4<i32> = (x_58 / vec4<i32>(2i, 2i, 2i, 2i));
  val = x_59;
  let x_60 : vec4<i32> = (x_59 * vec4<i32>(2i, 2i, 2i, 2i));
  val = x_60;
  let x_61 : vec4<i32> = (x_60 / vec4<i32>(2i, 2i, 2i, 2i));
  val = x_61;
  x_66_phi = false;
  if (x_55) {
    x_65 = all((x_61 == x_44));
    x_66_phi = x_65;
  }
  let x_66 : bool = x_66_phi;
  ok = x_66;
  return x_66;
}

fn main_1() {
  var outputColor_S0 : vec4<f32>;
  var output_S1 : vec4<f32>;
  var x_8_unknown : f32;
  var x_9_ok : bool;
  var x_10_val : vec4<f32>;
  var x_116 : vec4<f32>;
  var x_86 : bool;
  var x_99 : bool;
  var x_110 : bool;
  var x_114 : bool;
  var x_87_phi : bool;
  var x_100_phi : bool;
  var x_111_phi : bool;
  var x_115_phi : bool;
  let x_72 : vec4<f32> = vcolor_S0;
  outputColor_S0 = x_72;
  let x_77 : f32 = x_4.unknownInput_S1_c0;
  x_8_unknown = x_77;
  x_9_ok = true;
  x_87_phi = false;
  if (true) {
    x_86 = all(((vec4<f32>(0.0, 0.0, 0.0, 0.0) / vec4<f32>(x_77, x_77, x_77, x_77)) == vec4<f32>(0.0, 0.0, 0.0, 0.0)));
    x_87_phi = x_86;
  }
  let x_87 : bool = x_87_phi;
  x_9_ok = x_87;
  let x_89 : vec4<f32> = vec4<f32>(x_77, x_77, x_77, x_77);
  x_10_val = x_89;
  let x_92 : vec4<f32> = (x_89 + vec4<f32>(1.0, 1.0, 1.0, 1.0));
  x_10_val = x_92;
  let x_93 : vec4<f32> = (x_92 - vec4<f32>(1.0, 1.0, 1.0, 1.0));
  x_10_val = x_93;
  let x_94 : vec4<f32> = (x_93 + vec4<f32>(1.0, 1.0, 1.0, 1.0));
  x_10_val = x_94;
  let x_95 : vec4<f32> = (x_94 - vec4<f32>(1.0, 1.0, 1.0, 1.0));
  x_10_val = x_95;
  x_100_phi = false;
  if (x_87) {
    x_99 = all((x_95 == x_89));
    x_100_phi = x_99;
  }
  let x_100 : bool = x_100_phi;
  x_9_ok = x_100;
  let x_103 : vec4<f32> = (x_95 * vec4<f32>(2.0, 2.0, 2.0, 2.0));
  x_10_val = x_103;
  let x_104 : vec4<f32> = (x_103 / vec4<f32>(2.0, 2.0, 2.0, 2.0));
  x_10_val = x_104;
  let x_105 : vec4<f32> = (x_104 * vec4<f32>(2.0, 2.0, 2.0, 2.0));
  x_10_val = x_105;
  let x_106 : vec4<f32> = (x_105 / vec4<f32>(2.0, 2.0, 2.0, 2.0));
  x_10_val = x_106;
  x_111_phi = false;
  if (x_100) {
    x_110 = all((x_106 == x_89));
    x_111_phi = x_110;
  }
  let x_111 : bool = x_111_phi;
  x_9_ok = x_111;
  x_115_phi = false;
  if (x_111) {
    x_114 = test_int_S1_c0_b();
    x_115_phi = x_114;
  }
  let x_115 : bool = x_115_phi;
  if (x_115) {
    let x_122 : vec4<f32> = x_4.ucolorGreen_S1_c0;
    x_116 = x_122;
  } else {
    let x_124 : vec4<f32> = x_4.ucolorRed_S1_c0;
    x_116 = x_124;
  }
  let x_125 : vec4<f32> = x_116;
  output_S1 = x_125;
  sk_FragColor = x_125;
  return;
}

struct main_out {
  @location(0)
  sk_FragColor_1 : vec4<f32>,
}

@stage(fragment)
fn main(@builtin(front_facing) sk_Clockwise_param : bool, @location(0) vcolor_S0_param : vec4<f32>) -> main_out {
  sk_Clockwise = sk_Clockwise_param;
  vcolor_S0 = vcolor_S0_param;
  main_1();
  return main_out(sk_FragColor);
}
