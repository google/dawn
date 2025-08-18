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
  var v_17 : f32;
  var v_18 : bool;
  var v_19 : vec4<f32>;
  var v_20 : vec4<f32>;
  outputColor_S0 = vcolor_S0;
  let v_21 = v.unknownInput_S1_c0;
  v_17 = v_21;
  v_18 = true;
  var v_22 : bool;
  if (true) {
    v_22 = all(((vec4<f32>() / vec4<f32>(v_21, v_21, v_21, v_21)) == vec4<f32>()));
  } else {
    v_22 = false;
  }
  let v_23 = v_22;
  v_18 = v_23;
  let v_24 = vec4<f32>(v_21, v_21, v_21, v_21);
  v_19 = v_24;
  let v_25 = (v_24 + vec4<f32>(1.0f));
  v_19 = v_25;
  let v_26 = (v_25 - vec4<f32>(1.0f));
  v_19 = v_26;
  let v_27 = (v_26 + vec4<f32>(1.0f));
  v_19 = v_27;
  let v_28 = (v_27 - vec4<f32>(1.0f));
  v_19 = v_28;
  var v_29 : bool;
  if (v_23) {
    v_29 = all((v_28 == v_24));
  } else {
    v_29 = false;
  }
  let v_30 = v_29;
  v_18 = v_30;
  let v_31 = (v_28 * vec4<f32>(2.0f));
  v_19 = v_31;
  let v_32 = (v_31 / vec4<f32>(2.0f));
  v_19 = v_32;
  let v_33 = (v_32 * vec4<f32>(2.0f));
  v_19 = v_33;
  let v_34 = (v_33 / vec4<f32>(2.0f));
  v_19 = v_34;
  var v_35 : bool;
  if (v_30) {
    v_35 = all((v_34 == v_24));
  } else {
    v_35 = false;
  }
  let v_36 = v_35;
  v_18 = v_36;
  var v_37 : bool;
  if (v_36) {
    v_37 = test_int_S1_c0_b();
  } else {
    v_37 = false;
  }
  if (v_37) {
    v_20 = v.ucolorGreen_S1_c0;
  } else {
    v_20 = v.ucolorRed_S1_c0;
  }
  let v_38 = v_20;
  output_S1 = v_38;
  sk_FragColor = v_38;
}

@fragment
fn main(@builtin(front_facing) sk_Clockwise : bool, @location(0u) vcolor_S0 : vec4<f32>) -> @location(0u) vec4<f32> {
  main_inner(sk_Clockwise, vcolor_S0);
  return sk_FragColor;
}
