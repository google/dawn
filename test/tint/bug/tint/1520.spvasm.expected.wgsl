struct UniformBuffer_1 {
  tint_pad_0 : u32,
  tint_pad_4 : u32,
  tint_pad_8 : u32,
  tint_pad_12 : u32,
  unknownInput_S1_c0 : f32,
  ucolorRed_S1_c0 : vec4<f32>,
  ucolorGreen_S1_c0 : vec4<f32>,
  umatrix_S1 : mat3x3<f32>,
}

@group(0u) @binding(0u) var<uniform> v : UniformBuffer_1;

var<private> sk_FragColor : vec4<f32>;

fn test_int_S1_c0_b() -> bool {
  var unknown : i32;
  var ok : bool;
  var val : vec4<i32>;
  let v_1 = i32(v.unknownInput_S1_c0);
  unknown = v_1;
  ok = true;
  var v_2 : bool;
  if (true) {
    v_2 = all(((vec4<i32>() / vec4<i32>(v_1, v_1, v_1, v_1)) == vec4<i32>()));
  } else {
    v_2 = false;
  }
  let v_3 = v_2;
  ok = v_3;
  let v_4 = vec4<i32>(v_1, v_1, v_1, v_1);
  val = v_4;
  let v_5 = (v_4 + vec4<i32>(1i));
  val = v_5;
  let v_6 = (v_5 - vec4<i32>(1i));
  val = v_6;
  let v_7 = (v_6 + vec4<i32>(1i));
  val = v_7;
  let v_8 = (v_7 - vec4<i32>(1i));
  val = v_8;
  var v_9 : bool;
  if (v_3) {
    v_9 = all((v_8 == v_4));
  } else {
    v_9 = false;
  }
  let v_10 = v_9;
  ok = v_10;
  let v_11 = (v_8 * vec4<i32>(2i));
  val = v_11;
  let v_12 = (v_11 / vec4<i32>(2i));
  val = v_12;
  let v_13 = (v_12 * vec4<i32>(2i));
  val = v_13;
  let v_14 = (v_13 / vec4<i32>(2i));
  val = v_14;
  var v_15 : bool;
  if (v_10) {
    v_15 = all((v_14 == v_4));
  } else {
    v_15 = false;
  }
  let v_16 = v_15;
  ok = v_16;
  return v_16;
}

fn main_inner(sk_Clockwise : bool, vcolor_S0 : vec4<f32>) {
  var outputColor_S0 : vec4<f32>;
  var output_S1 : vec4<f32>;
  var _8_unknown : f32;
  var _9_ok : bool;
  var _10_val : vec4<f32>;
  var v_17 : vec4<f32>;
  outputColor_S0 = vcolor_S0;
  let v_18 = v.unknownInput_S1_c0;
  _8_unknown = v_18;
  _9_ok = true;
  var v_19 : bool;
  if (true) {
    v_19 = all(((vec4<f32>() / vec4<f32>(v_18, v_18, v_18, v_18)) == vec4<f32>()));
  } else {
    v_19 = false;
  }
  let v_20 = v_19;
  _9_ok = v_20;
  let v_21 = vec4<f32>(v_18, v_18, v_18, v_18);
  _10_val = v_21;
  let v_22 = (v_21 + vec4<f32>(1.0f));
  _10_val = v_22;
  let v_23 = (v_22 - vec4<f32>(1.0f));
  _10_val = v_23;
  let v_24 = (v_23 + vec4<f32>(1.0f));
  _10_val = v_24;
  let v_25 = (v_24 - vec4<f32>(1.0f));
  _10_val = v_25;
  var v_26 : bool;
  if (v_20) {
    v_26 = all((v_25 == v_21));
  } else {
    v_26 = false;
  }
  let v_27 = v_26;
  _9_ok = v_27;
  let v_28 = (v_25 * vec4<f32>(2.0f));
  _10_val = v_28;
  let v_29 = (v_28 / vec4<f32>(2.0f));
  _10_val = v_29;
  let v_30 = (v_29 * vec4<f32>(2.0f));
  _10_val = v_30;
  let v_31 = (v_30 / vec4<f32>(2.0f));
  _10_val = v_31;
  var v_32 : bool;
  if (v_27) {
    v_32 = all((v_31 == v_21));
  } else {
    v_32 = false;
  }
  let v_33 = v_32;
  _9_ok = v_33;
  var v_34 : bool;
  if (v_33) {
    v_34 = test_int_S1_c0_b();
  } else {
    v_34 = false;
  }
  if (v_34) {
    v_17 = v.ucolorGreen_S1_c0;
  } else {
    v_17 = v.ucolorRed_S1_c0;
  }
  let v_35 = v_17;
  output_S1 = v_35;
  sk_FragColor = v_35;
}

@fragment
fn main(@builtin(front_facing) sk_Clockwise : bool, @location(0u) vcolor_S0 : vec4<f32>) -> @location(0u) vec4<f32> {
  main_inner(sk_Clockwise, vcolor_S0);
  return sk_FragColor;
}
