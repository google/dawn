alias RTArr = array<f32>;

alias RTArr_1 = array<f32>;

struct ResultMatrix {
  /* @offset(0) */
  numbers : RTArr_1,
}

alias RTArr_2 = array<f32>;

struct FirstMatrix {
  /* @offset(0) */
  numbers : RTArr_1,
}

struct SecondMatrix {
  /* @offset(0) */
  numbers : RTArr_1,
}

struct Uniforms {
  /* @offset(0) */
  NAN : f32,
  /* @offset(4) */
  sizeA : i32,
  /* @offset(8) */
  sizeB : i32,
}

var<private> gl_GlobalInvocationID : vec3u;

@group(0) @binding(2) var<storage, read_write> resultMatrix : ResultMatrix;

@group(0) @binding(0) var<storage, read> firstMatrix : FirstMatrix;

@group(0) @binding(1) var<storage, read> secondMatrix : SecondMatrix;

@group(0) @binding(3) var<uniform> x_46 : Uniforms;

fn binaryOperation_f1_f1_(a : ptr<function, f32>, b : ptr<function, f32>) -> f32 {
  var x_26 : f32;
  let x_13 : f32 = *(b);
  if ((x_13 == 0.0f)) {
    return 1.0f;
  }
  let x_21 : f32 = *(b);
  if (!((round((x_21 - (2.0f * floor((x_21 / 2.0f))))) == 1.0f))) {
    let x_29 : f32 = *(a);
    let x_31 : f32 = *(b);
    x_26 = pow(abs(x_29), x_31);
  } else {
    let x_34 : f32 = *(a);
    let x_36 : f32 = *(a);
    let x_38 : f32 = *(b);
    x_26 = (sign(x_34) * pow(abs(x_36), x_38));
  }
  let x_41 : f32 = x_26;
  return x_41;
}

fn main_1() {
  var index : i32;
  var a_1 : i32;
  var param : f32;
  var param_1 : f32;
  let x_54 : u32 = gl_GlobalInvocationID.x;
  index = bitcast<i32>(x_54);
  a_1 = -10i;
  let x_63 : i32 = index;
  param = -4.0f;
  param_1 = -3.0f;
  let x_68 : f32 = binaryOperation_f1_f1_(&(param), &(param_1));
  resultMatrix.numbers[x_63] = x_68;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main(@builtin(global_invocation_id) gl_GlobalInvocationID_param : vec3u) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}
