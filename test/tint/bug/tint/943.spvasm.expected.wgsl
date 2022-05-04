struct Uniforms {
  NAN : f32,
  @size(12)
  padding : u32,
  aShape : vec3<i32>,
  @size(4)
  padding_1 : u32,
  bShape : vec3<i32>,
  @size(4)
  padding_2 : u32,
  outShape : vec3<i32>,
  @size(4)
  padding_3 : u32,
  outShapeStrides : vec2<i32>,
}

type RTArr = array<f32>;

type RTArr_1 = array<f32>;

struct ssbOut {
  result : RTArr_1,
}

type RTArr_2 = array<f32>;

struct ssbA {
  A : RTArr_1,
}

struct ssbB {
  B : RTArr_1,
}

var<private> dimAOuter_1 : i32;

@group(0) @binding(3) var<uniform> x_48 : Uniforms;

var<private> dimInner_1 : i32;

var<private> dimBOuter_1 : i32;

@group(0) @binding(0) var<storage, read_write> x_54 : ssbOut;

var<private> gl_LocalInvocationID : vec3<u32>;

var<private> gl_GlobalInvocationID : vec3<u32>;

var<workgroup> mm_Asub : array<array<f32, 64u>, 64u>;

var<workgroup> mm_Bsub : array<array<f32, 1u>, 64u>;

@group(0) @binding(1) var<storage, read> x_165 : ssbA;

var<private> batch : i32;

@group(0) @binding(2) var<storage, read> x_185 : ssbB;

fn coordsInBounds_vi2_vi2_(coord : ptr<function, vec2<i32>>, shape : ptr<function, vec2<i32>>) -> bool {
  var x_87 : bool;
  var x_88_phi : bool;
  let x_76 : vec2<i32> = *(coord);
  let x_81 : bool = all((x_76 >= vec2<i32>(0i, 0i)));
  x_88_phi = x_81;
  if (x_81) {
    let x_84 : vec2<i32> = *(coord);
    let x_85 : vec2<i32> = *(shape);
    x_87 = all((x_84 < x_85));
    x_88_phi = x_87;
  }
  let x_88 : bool = x_88_phi;
  return x_88;
}

fn mm_readA_i1_i1_(row : ptr<function, i32>, col : ptr<function, i32>) -> f32 {
  var batchASize : i32;
  var param_10 : vec2<i32>;
  var param_11 : vec2<i32>;
  var x_430 : f32;
  let x_417 : i32 = x_48.aShape.y;
  let x_419 : i32 = x_48.aShape.z;
  batchASize = (x_417 * x_419);
  let x_421 : i32 = *(row);
  let x_422 : i32 = *(col);
  let x_424 : i32 = dimAOuter_1;
  let x_425 : i32 = dimInner_1;
  param_10 = vec2<i32>(x_421, x_422);
  param_11 = vec2<i32>(x_424, x_425);
  let x_429 : bool = coordsInBounds_vi2_vi2_(&(param_10), &(param_11));
  if (x_429) {
    let x_438 : i32 = batch;
    let x_439 : i32 = batchASize;
    let x_441 : i32 = *(row);
    let x_442 : i32 = dimInner_1;
    let x_445 : i32 = *(col);
    let x_448 : f32 = x_165.A[(((x_438 * x_439) + (x_441 * x_442)) + x_445)];
    x_430 = x_448;
  } else {
    x_430 = 0.0;
  }
  let x_450 : f32 = x_430;
  return x_450;
}

fn mm_readB_i1_i1_(row_1 : ptr<function, i32>, col_1 : ptr<function, i32>) -> f32 {
  var batchBSize : i32;
  var param_12 : vec2<i32>;
  var param_13 : vec2<i32>;
  var x_468 : f32;
  let x_455 : i32 = x_48.bShape.y;
  let x_457 : i32 = x_48.bShape.z;
  batchBSize = (x_455 * x_457);
  let x_459 : i32 = *(row_1);
  let x_460 : i32 = *(col_1);
  let x_462 : i32 = dimInner_1;
  let x_463 : i32 = dimBOuter_1;
  param_12 = vec2<i32>(x_459, x_460);
  param_13 = vec2<i32>(x_462, x_463);
  let x_467 : bool = coordsInBounds_vi2_vi2_(&(param_12), &(param_13));
  if (x_467) {
    let x_475 : i32 = batch;
    let x_476 : i32 = batchBSize;
    let x_478 : i32 = *(row_1);
    let x_479 : i32 = dimBOuter_1;
    let x_482 : i32 = *(col_1);
    let x_485 : f32 = x_185.B[(((x_475 * x_476) + (x_478 * x_479)) + x_482)];
    x_468 = x_485;
  } else {
    x_468 = 0.0;
  }
  let x_487 : f32 = x_468;
  return x_487;
}

fn getOutputFlatIndex_vi3_(coords : ptr<function, vec3<i32>>) -> i32 {
  let x_99 : vec3<i32> = *(coords);
  let x_105 : i32 = x_48.outShapeStrides.x;
  let x_107 : i32 = x_48.outShapeStrides.y;
  return i32(dot(vec3<f32>(x_99), vec3<f32>(vec3<i32>(x_105, x_107, 1i))));
}

fn setOutput_i1_f1_(flatIndex : ptr<function, i32>, value : ptr<function, f32>) {
  let x_95 : i32 = *(flatIndex);
  let x_96 : f32 = *(value);
  x_54.result[x_95] = x_96;
  return;
}

fn setOutput_i1_i1_i1_f1_(d0 : ptr<function, i32>, d1 : ptr<function, i32>, d2 : ptr<function, i32>, value_1 : ptr<function, f32>) {
  var flatIndex_1 : i32;
  var param : vec3<i32>;
  var param_1 : i32;
  var param_2 : f32;
  let x_115 : i32 = *(d0);
  let x_116 : i32 = *(d1);
  let x_117 : i32 = *(d2);
  param = vec3<i32>(x_115, x_116, x_117);
  let x_120 : i32 = getOutputFlatIndex_vi3_(&(param));
  flatIndex_1 = x_120;
  let x_122 : i32 = flatIndex_1;
  param_1 = x_122;
  let x_124 : f32 = *(value_1);
  param_2 = x_124;
  setOutput_i1_f1_(&(param_1), &(param_2));
  return;
}

fn mm_write_i1_i1_f1_(row_2 : ptr<function, i32>, col_2 : ptr<function, i32>, value_2 : ptr<function, f32>) {
  var outCoord : vec3<i32>;
  var param_14 : i32;
  var param_15 : i32;
  var param_16 : i32;
  var param_17 : f32;
  let x_491 : i32 = batch;
  let x_492 : i32 = *(row_2);
  let x_493 : i32 = *(col_2);
  outCoord = vec3<i32>(x_491, x_492, x_493);
  let x_496 : i32 = batch;
  param_14 = x_496;
  let x_498 : i32 = *(row_2);
  param_15 = x_498;
  let x_500 : i32 = *(col_2);
  param_16 = x_500;
  let x_502 : f32 = *(value_2);
  param_17 = x_502;
  setOutput_i1_i1_i1_f1_(&(param_14), &(param_15), &(param_16), &(param_17));
  return;
}

fn mm_matMul_i1_i1_i1_(dimAOuter : ptr<function, i32>, dimInner : ptr<function, i32>, dimBOuter : ptr<function, i32>) {
  var tileRow : i32;
  var tileCol : i32;
  var globalRow : i32;
  var globalCol : i32;
  var numTiles : i32;
  var innerRow : i32;
  var innerCol : i32;
  var acc : array<array<f32, 1u>, 1u>;
  var tileColA : i32;
  var tileRowB : i32;
  var t : i32;
  var innerRow_1 : i32;
  var innerCol_1 : i32;
  var inputRow : i32;
  var inputCol : i32;
  var param_3 : i32;
  var param_4 : i32;
  var innerRow_2 : i32;
  var innerCol_2 : i32;
  var inputRow_1 : i32;
  var inputCol_1 : i32;
  var param_5 : i32;
  var param_6 : i32;
  var k : i32;
  var inner : i32;
  var BCached : array<f32, 1u>;
  var innerRow_3 : i32;
  var ACached : f32;
  var innerCol_3 : i32;
  var innerRow_4 : i32;
  var innerCol_4 : i32;
  var param_7 : i32;
  var param_8 : i32;
  var param_9 : f32;
  let x_132 : u32 = gl_LocalInvocationID.y;
  tileRow = (bitcast<i32>(x_132) * 1i);
  let x_137 : u32 = gl_LocalInvocationID.x;
  tileCol = (bitcast<i32>(x_137) * 1i);
  let x_143 : u32 = gl_GlobalInvocationID.y;
  globalRow = (bitcast<i32>(x_143) * 1i);
  let x_148 : u32 = gl_GlobalInvocationID.x;
  globalCol = (bitcast<i32>(x_148) * 1i);
  let x_152 : i32 = *(dimInner);
  numTiles = (((x_152 - 1i) / 64i) + 1i);
  innerRow = 0i;
  loop {
    let x_163 : i32 = innerRow;
    if ((x_163 < 1i)) {
    } else {
      break;
    }
    innerCol = 0i;
    loop {
      let x_171 : i32 = innerCol;
      if ((x_171 < 1i)) {
      } else {
        break;
      }
      let x_177 : i32 = innerRow;
      let x_178 : i32 = innerCol;
      acc[x_177][x_178] = 0.0;

      continuing {
        let x_181 : i32 = innerCol;
        innerCol = (x_181 + 1i);
      }
    }

    continuing {
      let x_183 : i32 = innerRow;
      innerRow = (x_183 + 1i);
    }
  }
  let x_187 : u32 = gl_LocalInvocationID.x;
  tileColA = (bitcast<i32>(x_187) * 64i);
  let x_192 : u32 = gl_LocalInvocationID.y;
  tileRowB = (bitcast<i32>(x_192) * 1i);
  t = 0i;
  loop {
    let x_201 : i32 = t;
    let x_202 : i32 = numTiles;
    if ((x_201 < x_202)) {
    } else {
      break;
    }
    innerRow_1 = 0i;
    loop {
      let x_210 : i32 = innerRow_1;
      if ((x_210 < 1i)) {
      } else {
        break;
      }
      innerCol_1 = 0i;
      loop {
        let x_218 : i32 = innerCol_1;
        if ((x_218 < 64i)) {
        } else {
          break;
        }
        let x_221 : i32 = tileRow;
        let x_222 : i32 = innerRow_1;
        inputRow = (x_221 + x_222);
        let x_225 : i32 = tileColA;
        let x_226 : i32 = innerCol_1;
        inputCol = (x_225 + x_226);
        let x_233 : i32 = inputRow;
        let x_234 : i32 = inputCol;
        let x_235 : i32 = globalRow;
        let x_236 : i32 = innerRow_1;
        let x_238 : i32 = t;
        let x_240 : i32 = inputCol;
        param_3 = (x_235 + x_236);
        param_4 = ((x_238 * 64i) + x_240);
        let x_244 : f32 = mm_readA_i1_i1_(&(param_3), &(param_4));
        mm_Asub[x_233][x_234] = x_244;

        continuing {
          let x_247 : i32 = innerCol_1;
          innerCol_1 = (x_247 + 1i);
        }
      }

      continuing {
        let x_249 : i32 = innerRow_1;
        innerRow_1 = (x_249 + 1i);
      }
    }
    innerRow_2 = 0i;
    loop {
      let x_257 : i32 = innerRow_2;
      if ((x_257 < 1i)) {
      } else {
        break;
      }
      innerCol_2 = 0i;
      loop {
        let x_265 : i32 = innerCol_2;
        if ((x_265 < 1i)) {
        } else {
          break;
        }
        let x_268 : i32 = tileRowB;
        let x_269 : i32 = innerRow_2;
        inputRow_1 = (x_268 + x_269);
        let x_272 : i32 = tileCol;
        let x_273 : i32 = innerCol_2;
        inputCol_1 = (x_272 + x_273);
        let x_278 : i32 = inputRow_1;
        let x_279 : i32 = inputCol_1;
        let x_280 : i32 = t;
        let x_282 : i32 = inputRow_1;
        let x_284 : i32 = globalCol;
        let x_285 : i32 = innerCol_2;
        param_5 = ((x_280 * 64i) + x_282);
        param_6 = (x_284 + x_285);
        let x_289 : f32 = mm_readB_i1_i1_(&(param_5), &(param_6));
        mm_Bsub[x_278][x_279] = x_289;

        continuing {
          let x_291 : i32 = innerCol_2;
          innerCol_2 = (x_291 + 1i);
        }
      }

      continuing {
        let x_293 : i32 = innerRow_2;
        innerRow_2 = (x_293 + 1i);
      }
    }
    workgroupBarrier();
    k = 0i;
    loop {
      let x_302 : i32 = k;
      if ((x_302 < 64i)) {
      } else {
        break;
      }
      inner = 0i;
      loop {
        let x_310 : i32 = inner;
        if ((x_310 < 1i)) {
        } else {
          break;
        }
        let x_314 : i32 = inner;
        let x_315 : i32 = k;
        let x_316 : i32 = tileCol;
        let x_317 : i32 = inner;
        let x_320 : f32 = mm_Bsub[x_315][(x_316 + x_317)];
        BCached[x_314] = x_320;

        continuing {
          let x_322 : i32 = inner;
          inner = (x_322 + 1i);
        }
      }
      innerRow_3 = 0i;
      loop {
        let x_330 : i32 = innerRow_3;
        if ((x_330 < 1i)) {
        } else {
          break;
        }
        let x_333 : i32 = tileRow;
        let x_334 : i32 = innerRow_3;
        let x_336 : i32 = k;
        let x_338 : f32 = mm_Asub[(x_333 + x_334)][x_336];
        ACached = x_338;
        innerCol_3 = 0i;
        loop {
          let x_345 : i32 = innerCol_3;
          if ((x_345 < 1i)) {
          } else {
            break;
          }
          let x_347 : i32 = innerRow_3;
          let x_348 : i32 = innerCol_3;
          let x_349 : f32 = ACached;
          let x_350 : i32 = innerCol_3;
          let x_352 : f32 = BCached[x_350];
          let x_355 : f32 = acc[x_347][x_348];
          acc[x_347][x_348] = (x_355 + (x_349 * x_352));

          continuing {
            let x_358 : i32 = innerCol_3;
            innerCol_3 = (x_358 + 1i);
          }
        }

        continuing {
          let x_360 : i32 = innerRow_3;
          innerRow_3 = (x_360 + 1i);
        }
      }

      continuing {
        let x_362 : i32 = k;
        k = (x_362 + 1i);
      }
    }
    workgroupBarrier();

    continuing {
      let x_364 : i32 = t;
      t = (x_364 + 1i);
    }
  }
  innerRow_4 = 0i;
  loop {
    let x_372 : i32 = innerRow_4;
    if ((x_372 < 1i)) {
    } else {
      break;
    }
    innerCol_4 = 0i;
    loop {
      var x_393 : bool;
      var x_394_phi : bool;
      let x_380 : i32 = innerCol_4;
      if ((x_380 < 1i)) {
      } else {
        break;
      }
      let x_382 : i32 = globalCol;
      let x_383 : i32 = innerCol_4;
      let x_385 : i32 = *(dimBOuter);
      let x_386 : bool = ((x_382 + x_383) < x_385);
      x_394_phi = x_386;
      if (x_386) {
        let x_389 : i32 = globalRow;
        let x_390 : i32 = innerRow_4;
        let x_392 : i32 = *(dimAOuter);
        x_393 = ((x_389 + x_390) < x_392);
        x_394_phi = x_393;
      }
      let x_394 : bool = x_394_phi;
      if (x_394) {
        let x_397 : i32 = globalRow;
        let x_398 : i32 = innerRow_4;
        let x_400 : i32 = globalCol;
        let x_401 : i32 = innerCol_4;
        let x_403 : i32 = innerRow_4;
        let x_404 : i32 = innerCol_4;
        param_7 = (x_397 + x_398);
        param_8 = (x_400 + x_401);
        let x_409 : f32 = acc[x_403][x_404];
        param_9 = x_409;
        mm_write_i1_i1_f1_(&(param_7), &(param_8), &(param_9));
      }

      continuing {
        let x_411 : i32 = innerCol_4;
        innerCol_4 = (x_411 + 1i);
      }
    }

    continuing {
      let x_413 : i32 = innerRow_4;
      innerRow_4 = (x_413 + 1i);
    }
  }
  return;
}

fn main_1() {
  var param_18 : i32;
  var param_19 : i32;
  var param_20 : i32;
  let x_67 : i32 = x_48.aShape.y;
  dimAOuter_1 = x_67;
  let x_71 : i32 = x_48.aShape.z;
  dimInner_1 = x_71;
  let x_75 : i32 = x_48.bShape.z;
  dimBOuter_1 = x_75;
  let x_505 : u32 = gl_GlobalInvocationID.z;
  batch = bitcast<i32>(x_505);
  let x_508 : i32 = dimAOuter_1;
  param_18 = x_508;
  let x_510 : i32 = dimInner_1;
  param_19 = x_510;
  let x_512 : i32 = dimBOuter_1;
  param_20 = x_512;
  mm_matMul_i1_i1_i1_(&(param_18), &(param_19), &(param_20));
  return;
}

@stage(compute) @workgroup_size(1, 64, 1)
fn main(@builtin(local_invocation_id) gl_LocalInvocationID_param : vec3<u32>, @builtin(global_invocation_id) gl_GlobalInvocationID_param : vec3<u32>) {
  gl_LocalInvocationID = gl_LocalInvocationID_param;
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}
