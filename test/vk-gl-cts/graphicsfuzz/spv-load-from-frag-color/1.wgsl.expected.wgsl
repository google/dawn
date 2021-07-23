struct BST {
  data : i32;
  leftIndex : i32;
  rightIndex : i32;
};

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var tree : array<BST, 10>;
  var x_360 : i32;
  var x_62_phi : i32;
  var x_90_phi : bool;
  var x_357_phi : i32;
  var x_360_phi : i32;
  var x_362_phi : i32;
  tree[0] = BST(9, -1, -1);
  switch(0u) {
    default: {
      x_62_phi = 0;
      loop {
        var x_88 : i32;
        var x_80 : i32;
        var x_63 : i32;
        var x_63_phi : i32;
        let x_62 : i32 = x_62_phi;
        x_90_phi = false;
        if ((x_62 <= 1)) {
        } else {
          break;
        }
        let x_69 : i32 = tree[x_62].data;
        if ((5 <= x_69)) {
          let x_82 : ptr<function, i32> = &(tree[x_62].leftIndex);
          let x_83 : i32 = *(x_82);
          if ((x_83 == -1)) {
            *(x_82) = 1;
            tree[1] = BST(5, -1, -1);
            x_90_phi = true;
            break;
          } else {
            x_88 = *(x_82);
            x_63_phi = x_88;
            continue;
          }
          return;
        } else {
          let x_74 : ptr<function, i32> = &(tree[x_62].rightIndex);
          let x_75 : i32 = *(x_74);
          if ((x_75 == -1)) {
            *(x_74) = 1;
            tree[1] = BST(5, -1, -1);
            x_90_phi = true;
            break;
          } else {
            x_80 = *(x_74);
            x_63_phi = x_80;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_63 = x_63_phi;
          x_62_phi = x_63;
        }
      }
      let x_90 : bool = x_90_phi;
      if (x_90) {
        break;
      }
    }
  }
  var x_95_phi : i32;
  var x_123_phi : bool;
  switch(0u) {
    default: {
      x_95_phi = 0;
      loop {
        var x_121 : i32;
        var x_113 : i32;
        var x_96 : i32;
        var x_96_phi : i32;
        let x_95 : i32 = x_95_phi;
        x_123_phi = false;
        if ((x_95 <= 2)) {
        } else {
          break;
        }
        let x_102 : i32 = tree[x_95].data;
        if ((12 <= x_102)) {
          let x_115 : ptr<function, i32> = &(tree[x_95].leftIndex);
          let x_116 : i32 = *(x_115);
          if ((x_116 == -1)) {
            *(x_115) = 2;
            tree[2] = BST(12, -1, -1);
            x_123_phi = true;
            break;
          } else {
            x_121 = *(x_115);
            x_96_phi = x_121;
            continue;
          }
          return;
        } else {
          let x_107 : ptr<function, i32> = &(tree[x_95].rightIndex);
          let x_108 : i32 = *(x_107);
          if ((x_108 == -1)) {
            *(x_107) = 2;
            tree[2] = BST(12, -1, -1);
            x_123_phi = true;
            break;
          } else {
            x_113 = *(x_107);
            x_96_phi = x_113;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_96 = x_96_phi;
          x_95_phi = x_96;
        }
      }
      let x_123 : bool = x_123_phi;
      if (x_123) {
        break;
      }
    }
  }
  var x_128_phi : i32;
  var x_156_phi : bool;
  switch(0u) {
    default: {
      x_128_phi = 0;
      loop {
        var x_154 : i32;
        var x_146 : i32;
        var x_129 : i32;
        var x_129_phi : i32;
        let x_128 : i32 = x_128_phi;
        x_156_phi = false;
        if ((x_128 <= 3)) {
        } else {
          break;
        }
        let x_135 : i32 = tree[x_128].data;
        if ((15 <= x_135)) {
          let x_148 : ptr<function, i32> = &(tree[x_128].leftIndex);
          let x_149 : i32 = *(x_148);
          if ((x_149 == -1)) {
            *(x_148) = 3;
            tree[3] = BST(15, -1, -1);
            x_156_phi = true;
            break;
          } else {
            x_154 = *(x_148);
            x_129_phi = x_154;
            continue;
          }
          return;
        } else {
          let x_140 : ptr<function, i32> = &(tree[x_128].rightIndex);
          let x_141 : i32 = *(x_140);
          if ((x_141 == -1)) {
            *(x_140) = 3;
            tree[3] = BST(15, -1, -1);
            x_156_phi = true;
            break;
          } else {
            x_146 = *(x_140);
            x_129_phi = x_146;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_129 = x_129_phi;
          x_128_phi = x_129;
        }
      }
      let x_156 : bool = x_156_phi;
      if (x_156) {
        break;
      }
    }
  }
  var x_161_phi : i32;
  var x_189_phi : bool;
  switch(0u) {
    default: {
      x_161_phi = 0;
      loop {
        var x_187 : i32;
        var x_179 : i32;
        var x_162 : i32;
        var x_162_phi : i32;
        let x_161 : i32 = x_161_phi;
        x_189_phi = false;
        if ((x_161 <= 4)) {
        } else {
          break;
        }
        let x_168 : i32 = tree[x_161].data;
        if ((7 <= x_168)) {
          let x_181 : ptr<function, i32> = &(tree[x_161].leftIndex);
          let x_182 : i32 = *(x_181);
          if ((x_182 == -1)) {
            *(x_181) = 4;
            tree[4] = BST(7, -1, -1);
            x_189_phi = true;
            break;
          } else {
            x_187 = *(x_181);
            x_162_phi = x_187;
            continue;
          }
          return;
        } else {
          let x_173 : ptr<function, i32> = &(tree[x_161].rightIndex);
          let x_174 : i32 = *(x_173);
          if ((x_174 == -1)) {
            *(x_173) = 4;
            tree[4] = BST(7, -1, -1);
            x_189_phi = true;
            break;
          } else {
            x_179 = *(x_173);
            x_162_phi = x_179;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_162 = x_162_phi;
          x_161_phi = x_162;
        }
      }
      let x_189 : bool = x_189_phi;
      if (x_189) {
        break;
      }
    }
  }
  var x_194_phi : i32;
  var x_222_phi : bool;
  switch(0u) {
    default: {
      x_194_phi = 0;
      loop {
        var x_220 : i32;
        var x_212 : i32;
        var x_195 : i32;
        var x_195_phi : i32;
        let x_194 : i32 = x_194_phi;
        x_222_phi = false;
        if ((x_194 <= 5)) {
        } else {
          break;
        }
        let x_201 : i32 = tree[x_194].data;
        if ((8 <= x_201)) {
          let x_214 : ptr<function, i32> = &(tree[x_194].leftIndex);
          let x_215 : i32 = *(x_214);
          if ((x_215 == -1)) {
            *(x_214) = 5;
            tree[5] = BST(8, -1, -1);
            x_222_phi = true;
            break;
          } else {
            x_220 = *(x_214);
            x_195_phi = x_220;
            continue;
          }
          return;
        } else {
          let x_206 : ptr<function, i32> = &(tree[x_194].rightIndex);
          let x_207 : i32 = *(x_206);
          if ((x_207 == -1)) {
            *(x_206) = 5;
            tree[5] = BST(8, -1, -1);
            x_222_phi = true;
            break;
          } else {
            x_212 = *(x_206);
            x_195_phi = x_212;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_195 = x_195_phi;
          x_194_phi = x_195;
        }
      }
      let x_222 : bool = x_222_phi;
      if (x_222) {
        break;
      }
    }
  }
  var x_227_phi : i32;
  var x_255_phi : bool;
  switch(0u) {
    default: {
      x_227_phi = 0;
      loop {
        var x_253 : i32;
        var x_245 : i32;
        var x_228 : i32;
        var x_228_phi : i32;
        let x_227 : i32 = x_227_phi;
        x_255_phi = false;
        if ((x_227 <= 6)) {
        } else {
          break;
        }
        let x_234 : i32 = tree[x_227].data;
        if ((2 <= x_234)) {
          let x_247 : ptr<function, i32> = &(tree[x_227].leftIndex);
          let x_248 : i32 = *(x_247);
          if ((x_248 == -1)) {
            *(x_247) = 6;
            tree[6] = BST(2, -1, -1);
            x_255_phi = true;
            break;
          } else {
            x_253 = *(x_247);
            x_228_phi = x_253;
            continue;
          }
          return;
        } else {
          let x_239 : ptr<function, i32> = &(tree[x_227].rightIndex);
          let x_240 : i32 = *(x_239);
          if ((x_240 == -1)) {
            *(x_239) = 6;
            tree[6] = BST(2, -1, -1);
            x_255_phi = true;
            break;
          } else {
            x_245 = *(x_239);
            x_228_phi = x_245;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_228 = x_228_phi;
          x_227_phi = x_228;
        }
      }
      let x_255 : bool = x_255_phi;
      if (x_255) {
        break;
      }
    }
  }
  var x_260_phi : i32;
  var x_288_phi : bool;
  switch(0u) {
    default: {
      x_260_phi = 0;
      loop {
        var x_286 : i32;
        var x_278 : i32;
        var x_261 : i32;
        var x_261_phi : i32;
        let x_260 : i32 = x_260_phi;
        x_288_phi = false;
        if ((x_260 <= 7)) {
        } else {
          break;
        }
        let x_267 : i32 = tree[x_260].data;
        if ((6 <= x_267)) {
          let x_280 : ptr<function, i32> = &(tree[x_260].leftIndex);
          let x_281 : i32 = *(x_280);
          if ((x_281 == -1)) {
            *(x_280) = 7;
            tree[7] = BST(6, -1, -1);
            x_288_phi = true;
            break;
          } else {
            x_286 = *(x_280);
            x_261_phi = x_286;
            continue;
          }
          return;
        } else {
          let x_272 : ptr<function, i32> = &(tree[x_260].rightIndex);
          let x_273 : i32 = *(x_272);
          if ((x_273 == -1)) {
            *(x_272) = 7;
            tree[7] = BST(6, -1, -1);
            x_288_phi = true;
            break;
          } else {
            x_278 = *(x_272);
            x_261_phi = x_278;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_261 = x_261_phi;
          x_260_phi = x_261;
        }
      }
      let x_288 : bool = x_288_phi;
      if (x_288) {
        break;
      }
    }
  }
  var x_293_phi : i32;
  var x_321_phi : bool;
  switch(0u) {
    default: {
      x_293_phi = 0;
      loop {
        var x_319 : i32;
        var x_311 : i32;
        var x_294 : i32;
        var x_294_phi : i32;
        let x_293 : i32 = x_293_phi;
        x_321_phi = false;
        if ((x_293 <= 8)) {
        } else {
          break;
        }
        let x_300 : i32 = tree[x_293].data;
        if ((17 <= x_300)) {
          let x_313 : ptr<function, i32> = &(tree[x_293].leftIndex);
          let x_314 : i32 = *(x_313);
          if ((x_314 == -1)) {
            *(x_313) = 8;
            tree[8] = BST(17, -1, -1);
            x_321_phi = true;
            break;
          } else {
            x_319 = *(x_313);
            x_294_phi = x_319;
            continue;
          }
          return;
        } else {
          let x_305 : ptr<function, i32> = &(tree[x_293].rightIndex);
          let x_306 : i32 = *(x_305);
          if ((x_306 == -1)) {
            *(x_305) = 8;
            tree[8] = BST(17, -1, -1);
            x_321_phi = true;
            break;
          } else {
            x_311 = *(x_305);
            x_294_phi = x_311;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_294 = x_294_phi;
          x_293_phi = x_294;
        }
      }
      let x_321 : bool = x_321_phi;
      if (x_321) {
        break;
      }
    }
  }
  var x_326_phi : i32;
  var x_354_phi : bool;
  switch(0u) {
    default: {
      x_326_phi = 0;
      loop {
        var x_352 : i32;
        var x_344 : i32;
        var x_327 : i32;
        var x_327_phi : i32;
        let x_326 : i32 = x_326_phi;
        x_354_phi = false;
        if ((x_326 <= 9)) {
        } else {
          break;
        }
        let x_333 : i32 = tree[x_326].data;
        if ((13 <= x_333)) {
          let x_346 : ptr<function, i32> = &(tree[x_326].leftIndex);
          let x_347 : i32 = *(x_346);
          if ((x_347 == -1)) {
            *(x_346) = 9;
            tree[9] = BST(13, -1, -1);
            x_354_phi = true;
            break;
          } else {
            x_352 = *(x_346);
            x_327_phi = x_352;
            continue;
          }
          return;
        } else {
          let x_338 : ptr<function, i32> = &(tree[x_326].rightIndex);
          let x_339 : i32 = *(x_338);
          if ((x_339 == -1)) {
            *(x_338) = 9;
            tree[9] = BST(13, -1, -1);
            x_354_phi = true;
            break;
          } else {
            x_344 = *(x_338);
            x_327_phi = x_344;
            continue;
          }
          return;
        }
        return;

        continuing {
          x_327 = x_327_phi;
          x_326_phi = x_327;
        }
      }
      let x_354 : bool = x_354_phi;
      if (x_354) {
        break;
      }
    }
  }
  x_357_phi = 0;
  x_360_phi = 0;
  x_362_phi = 0;
  loop {
    var x_392 : i32;
    var x_402 : i32;
    var x_407 : i32;
    var x_363 : i32;
    var x_358_phi : i32;
    var x_361_phi : i32;
    let x_357 : i32 = x_357_phi;
    x_360 = x_360_phi;
    let x_362 : i32 = x_362_phi;
    let x_365 : i32 = (6 - 15);
    if ((x_362 < 20)) {
    } else {
      break;
    }
    var x_374_phi : i32;
    var x_392_phi : i32;
    var x_393_phi : bool;
    switch(0u) {
      default: {
        x_374_phi = 0;
        loop {
          let x_374 : i32 = x_374_phi;
          x_392_phi = x_357;
          x_393_phi = false;
          if ((x_374 != -1)) {
          } else {
            break;
          }
          let x_381 : BST = tree[x_374];
          let x_382 : i32 = x_381.data;
          let x_383 : i32 = x_381.leftIndex;
          let x_385 : i32 = x_381.rightIndex;
          if ((x_382 == x_362)) {
            x_392_phi = x_362;
            x_393_phi = true;
            break;
          }
          let x_389 : f32 = x_GLF_color[select(3u, 3u, (3u <= 3u))];

          continuing {
            x_374_phi = select(x_383, x_385, !((x_362 <= x_382)));
          }
        }
        x_392 = x_392_phi;
        let x_393 : bool = x_393_phi;
        x_358_phi = x_392;
        if (x_393) {
          break;
        }
        x_358_phi = -1;
      }
    }
    var x_358 : i32;
    var x_401 : i32;
    var x_406 : i32;
    var x_402_phi : i32;
    var x_407_phi : i32;
    x_358 = x_358_phi;
    switch(x_362) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        x_402_phi = x_360;
        if ((x_358 == bitcast<i32>(x_362))) {
          x_401 = bitcast<i32>((x_360 + bitcast<i32>(1)));
          x_402_phi = x_401;
        }
        x_402 = x_402_phi;
        x_361_phi = x_402;
      }
      default: {
        x_407_phi = x_360;
        if ((x_358 == bitcast<i32>(-1))) {
          x_406 = bitcast<i32>((x_360 + bitcast<i32>(1)));
          x_407_phi = x_406;
        }
        x_407 = x_407_phi;
        x_361_phi = x_407;
      }
    }
    let x_361 : i32 = x_361_phi;

    continuing {
      x_363 = (x_362 + 1);
      x_357_phi = x_358;
      x_360_phi = x_361;
      x_362_phi = x_363;
    }
  }
  if ((x_360 == bitcast<i32>(20))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 1.0, 1.0);
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
