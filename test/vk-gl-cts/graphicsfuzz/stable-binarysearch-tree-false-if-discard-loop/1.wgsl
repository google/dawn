struct BST {
  data : i32;
  leftIndex : i32;
  rightIndex : i32;
};

[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_8 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var tree : array<BST, 10>;
  var x_67 : bool;
  var x_114 : bool;
  var x_572 : i32;
  var x_67_phi : bool;
  var x_70_phi : i32;
  var x_116_phi : bool;
  var x_119_phi : i32;
  var x_569_phi : i32;
  var x_572_phi : i32;
  var x_574_phi : i32;
  tree[0] = BST(9, -1, -1);
  switch(0u) {
    default: {
      x_67_phi = false;
      x_70_phi = 0;
      loop {
        var x_95 : i32;
        var x_87 : i32;
        var x_68 : bool;
        var x_71 : i32;
        var x_68_phi : bool;
        var x_71_phi : i32;
        x_67 = x_67_phi;
        let x_70 : i32 = x_70_phi;
        x_116_phi = x_67;
        if ((x_70 <= 1)) {
        } else {
          break;
        }
        let x_76 : i32 = tree[x_70].data;
        if ((5 <= x_76)) {
          var x_114_phi : bool;
          let x_89 : ptr<function, i32> = &(tree[x_70].leftIndex);
          let x_90 : i32 = *(x_89);
          if ((x_90 == -1)) {
            let x_97 : f32 = x_8.injectionSwitch.y;
            let x_99 : f32 = x_8.injectionSwitch.x;
            if ((x_97 < x_99)) {
              loop {
                discard;
              }
              return;
            }
            *(x_89) = 1;
            tree[1] = BST(5, -1, -1);
            loop {
              x_114_phi = x_67;
              if ((0 < i32(x_97))) {
              } else {
                break;
              }
              x_114_phi = true;
              break;
            }
            x_114 = x_114_phi;
            x_116_phi = x_114;
            if (x_114) {
              break;
            }
          } else {
            x_95 = *(x_89);
            x_68_phi = x_67;
            x_71_phi = x_95;
            continue;
          }
        } else {
          let x_81 : ptr<function, i32> = &(tree[x_70].rightIndex);
          let x_82 : i32 = *(x_81);
          if ((x_82 == -1)) {
            *(x_81) = 1;
            tree[1] = BST(5, -1, -1);
            x_116_phi = true;
            break;
          } else {
            x_87 = *(x_81);
            x_68_phi = x_67;
            x_71_phi = x_87;
            continue;
          }
          return;
        }
        x_68_phi = x_114;
        x_71_phi = x_70;

        continuing {
          x_68 = x_68_phi;
          x_71 = x_71_phi;
          x_67_phi = x_68;
          x_70_phi = x_71;
        }
      }
      let x_116 : bool = x_116_phi;
      if (x_116) {
        break;
      }
    }
  }
  x_119_phi = 0;
  loop {
    var x_133 : bool;
    var x_120 : i32;
    var x_134_phi : bool;
    let x_119 : i32 = x_119_phi;
    let x_125 : f32 = gl_FragCoord.y;
    let x_126 : bool = (x_125 < 0.0);
    x_134_phi = x_126;
    if (!(x_126)) {
      let x_131 : f32 = x_8.injectionSwitch.y;
      x_133 = (x_119 != i32(x_131));
      x_134_phi = x_133;
    }
    let x_134 : bool = x_134_phi;
    if (x_134) {
    } else {
      break;
    }
    var x_139 : bool;
    var x_186 : bool;
    var x_139_phi : bool;
    var x_142_phi : i32;
    var x_188_phi : bool;
    switch(0u) {
      default: {
        x_139_phi = false;
        x_142_phi = 0;
        loop {
          var x_167 : i32;
          var x_159 : i32;
          var x_140 : bool;
          var x_143 : i32;
          var x_140_phi : bool;
          var x_143_phi : i32;
          x_139 = x_139_phi;
          let x_142 : i32 = x_142_phi;
          x_188_phi = x_139;
          if ((x_142 <= 2)) {
          } else {
            break;
          }
          let x_148 : i32 = tree[x_142].data;
          if ((12 <= x_148)) {
            var x_186_phi : bool;
            let x_161 : ptr<function, i32> = &(tree[x_142].leftIndex);
            let x_162 : i32 = *(x_161);
            if ((x_162 == -1)) {
              let x_169 : f32 = x_8.injectionSwitch.y;
              let x_171 : f32 = x_8.injectionSwitch.x;
              if ((x_169 < x_171)) {
                loop {
                  discard;
                }
                return;
              }
              *(x_161) = 2;
              tree[2] = BST(12, -1, -1);
              loop {
                x_186_phi = x_139;
                if ((0 < i32(x_169))) {
                } else {
                  break;
                }
                x_186_phi = true;
                break;
              }
              x_186 = x_186_phi;
              x_188_phi = x_186;
              if (x_186) {
                break;
              }
            } else {
              x_167 = *(x_161);
              x_140_phi = x_139;
              x_143_phi = x_167;
              continue;
            }
          } else {
            let x_153 : ptr<function, i32> = &(tree[x_142].rightIndex);
            let x_154 : i32 = *(x_153);
            if ((x_154 == -1)) {
              *(x_153) = 2;
              tree[2] = BST(12, -1, -1);
              x_188_phi = true;
              break;
            } else {
              x_159 = *(x_153);
              x_140_phi = x_139;
              x_143_phi = x_159;
              continue;
            }
            return;
          }
          x_140_phi = x_186;
          x_143_phi = x_142;

          continuing {
            x_140 = x_140_phi;
            x_143 = x_143_phi;
            x_139_phi = x_140;
            x_142_phi = x_143;
          }
        }
        let x_188 : bool = x_188_phi;
        if (x_188) {
          break;
        }
      }
    }

    continuing {
      x_120 = (x_119 + 1);
      x_119_phi = x_120;
    }
  }
  var x_193 : bool;
  var x_240 : bool;
  var x_193_phi : bool;
  var x_196_phi : i32;
  var x_242_phi : bool;
  switch(0u) {
    default: {
      x_193_phi = false;
      x_196_phi = 0;
      loop {
        var x_221 : i32;
        var x_213 : i32;
        var x_194 : bool;
        var x_197 : i32;
        var x_194_phi : bool;
        var x_197_phi : i32;
        x_193 = x_193_phi;
        let x_196 : i32 = x_196_phi;
        x_242_phi = x_193;
        if ((x_196 <= 3)) {
        } else {
          break;
        }
        let x_202 : i32 = tree[x_196].data;
        if ((15 <= x_202)) {
          var x_240_phi : bool;
          let x_215 : ptr<function, i32> = &(tree[x_196].leftIndex);
          let x_216 : i32 = *(x_215);
          if ((x_216 == -1)) {
            let x_223 : f32 = x_8.injectionSwitch.y;
            let x_225 : f32 = x_8.injectionSwitch.x;
            if ((x_223 < x_225)) {
              loop {
                discard;
              }
              return;
            }
            *(x_215) = 3;
            tree[3] = BST(15, -1, -1);
            loop {
              x_240_phi = x_193;
              if ((0 < i32(x_223))) {
              } else {
                break;
              }
              x_240_phi = true;
              break;
            }
            x_240 = x_240_phi;
            x_242_phi = x_240;
            if (x_240) {
              break;
            }
          } else {
            x_221 = *(x_215);
            x_194_phi = x_193;
            x_197_phi = x_221;
            continue;
          }
        } else {
          let x_207 : ptr<function, i32> = &(tree[x_196].rightIndex);
          let x_208 : i32 = *(x_207);
          if ((x_208 == -1)) {
            *(x_207) = 3;
            tree[3] = BST(15, -1, -1);
            x_242_phi = true;
            break;
          } else {
            x_213 = *(x_207);
            x_194_phi = x_193;
            x_197_phi = x_213;
            continue;
          }
          return;
        }
        x_194_phi = x_240;
        x_197_phi = x_196;

        continuing {
          x_194 = x_194_phi;
          x_197 = x_197_phi;
          x_193_phi = x_194;
          x_196_phi = x_197;
        }
      }
      let x_242 : bool = x_242_phi;
      if (x_242) {
        break;
      }
    }
  }
  var x_247 : bool;
  var x_294 : bool;
  var x_247_phi : bool;
  var x_250_phi : i32;
  var x_296_phi : bool;
  switch(0u) {
    default: {
      x_247_phi = false;
      x_250_phi = 0;
      loop {
        var x_275 : i32;
        var x_267 : i32;
        var x_248 : bool;
        var x_251 : i32;
        var x_248_phi : bool;
        var x_251_phi : i32;
        x_247 = x_247_phi;
        let x_250 : i32 = x_250_phi;
        x_296_phi = x_247;
        if ((x_250 <= 4)) {
        } else {
          break;
        }
        let x_256 : i32 = tree[x_250].data;
        if ((7 <= x_256)) {
          var x_294_phi : bool;
          let x_269 : ptr<function, i32> = &(tree[x_250].leftIndex);
          let x_270 : i32 = *(x_269);
          if ((x_270 == -1)) {
            let x_277 : f32 = x_8.injectionSwitch.y;
            let x_279 : f32 = x_8.injectionSwitch.x;
            if ((x_277 < x_279)) {
              loop {
                discard;
              }
              return;
            }
            *(x_269) = 4;
            tree[4] = BST(7, -1, -1);
            loop {
              x_294_phi = x_247;
              if ((0 < i32(x_277))) {
              } else {
                break;
              }
              x_294_phi = true;
              break;
            }
            x_294 = x_294_phi;
            x_296_phi = x_294;
            if (x_294) {
              break;
            }
          } else {
            x_275 = *(x_269);
            x_248_phi = x_247;
            x_251_phi = x_275;
            continue;
          }
        } else {
          let x_261 : ptr<function, i32> = &(tree[x_250].rightIndex);
          let x_262 : i32 = *(x_261);
          if ((x_262 == -1)) {
            *(x_261) = 4;
            tree[4] = BST(7, -1, -1);
            x_296_phi = true;
            break;
          } else {
            x_267 = *(x_261);
            x_248_phi = x_247;
            x_251_phi = x_267;
            continue;
          }
          return;
        }
        x_248_phi = x_294;
        x_251_phi = x_250;

        continuing {
          x_248 = x_248_phi;
          x_251 = x_251_phi;
          x_247_phi = x_248;
          x_250_phi = x_251;
        }
      }
      let x_296 : bool = x_296_phi;
      if (x_296) {
        break;
      }
    }
  }
  var x_301 : bool;
  var x_348 : bool;
  var x_301_phi : bool;
  var x_304_phi : i32;
  var x_350_phi : bool;
  switch(0u) {
    default: {
      x_301_phi = false;
      x_304_phi = 0;
      loop {
        var x_329 : i32;
        var x_321 : i32;
        var x_302 : bool;
        var x_305 : i32;
        var x_302_phi : bool;
        var x_305_phi : i32;
        x_301 = x_301_phi;
        let x_304 : i32 = x_304_phi;
        x_350_phi = x_301;
        if ((x_304 <= 5)) {
        } else {
          break;
        }
        let x_310 : i32 = tree[x_304].data;
        if ((8 <= x_310)) {
          var x_348_phi : bool;
          let x_323 : ptr<function, i32> = &(tree[x_304].leftIndex);
          let x_324 : i32 = *(x_323);
          if ((x_324 == -1)) {
            let x_331 : f32 = x_8.injectionSwitch.y;
            let x_333 : f32 = x_8.injectionSwitch.x;
            if ((x_331 < x_333)) {
              loop {
                discard;
              }
              return;
            }
            *(x_323) = 5;
            tree[5] = BST(8, -1, -1);
            loop {
              x_348_phi = x_301;
              if ((0 < i32(x_331))) {
              } else {
                break;
              }
              x_348_phi = true;
              break;
            }
            x_348 = x_348_phi;
            x_350_phi = x_348;
            if (x_348) {
              break;
            }
          } else {
            x_329 = *(x_323);
            x_302_phi = x_301;
            x_305_phi = x_329;
            continue;
          }
        } else {
          let x_315 : ptr<function, i32> = &(tree[x_304].rightIndex);
          let x_316 : i32 = *(x_315);
          if ((x_316 == -1)) {
            *(x_315) = 5;
            tree[5] = BST(8, -1, -1);
            x_350_phi = true;
            break;
          } else {
            x_321 = *(x_315);
            x_302_phi = x_301;
            x_305_phi = x_321;
            continue;
          }
          return;
        }
        x_302_phi = x_348;
        x_305_phi = x_304;

        continuing {
          x_302 = x_302_phi;
          x_305 = x_305_phi;
          x_301_phi = x_302;
          x_304_phi = x_305;
        }
      }
      let x_350 : bool = x_350_phi;
      if (x_350) {
        break;
      }
    }
  }
  var x_355 : bool;
  var x_402 : bool;
  var x_355_phi : bool;
  var x_358_phi : i32;
  var x_404_phi : bool;
  switch(0u) {
    default: {
      x_355_phi = false;
      x_358_phi = 0;
      loop {
        var x_383 : i32;
        var x_375 : i32;
        var x_356 : bool;
        var x_359 : i32;
        var x_356_phi : bool;
        var x_359_phi : i32;
        x_355 = x_355_phi;
        let x_358 : i32 = x_358_phi;
        x_404_phi = x_355;
        if ((x_358 <= 6)) {
        } else {
          break;
        }
        let x_364 : i32 = tree[x_358].data;
        if ((2 <= x_364)) {
          var x_402_phi : bool;
          let x_377 : ptr<function, i32> = &(tree[x_358].leftIndex);
          let x_378 : i32 = *(x_377);
          if ((x_378 == -1)) {
            let x_385 : f32 = x_8.injectionSwitch.y;
            let x_387 : f32 = x_8.injectionSwitch.x;
            if ((x_385 < x_387)) {
              loop {
                discard;
              }
              return;
            }
            *(x_377) = 6;
            tree[6] = BST(2, -1, -1);
            loop {
              x_402_phi = x_355;
              if ((0 < i32(x_385))) {
              } else {
                break;
              }
              x_402_phi = true;
              break;
            }
            x_402 = x_402_phi;
            x_404_phi = x_402;
            if (x_402) {
              break;
            }
          } else {
            x_383 = *(x_377);
            x_356_phi = x_355;
            x_359_phi = x_383;
            continue;
          }
        } else {
          let x_369 : ptr<function, i32> = &(tree[x_358].rightIndex);
          let x_370 : i32 = *(x_369);
          if ((x_370 == -1)) {
            *(x_369) = 6;
            tree[6] = BST(2, -1, -1);
            x_404_phi = true;
            break;
          } else {
            x_375 = *(x_369);
            x_356_phi = x_355;
            x_359_phi = x_375;
            continue;
          }
          return;
        }
        x_356_phi = x_402;
        x_359_phi = x_358;

        continuing {
          x_356 = x_356_phi;
          x_359 = x_359_phi;
          x_355_phi = x_356;
          x_358_phi = x_359;
        }
      }
      let x_404 : bool = x_404_phi;
      if (x_404) {
        break;
      }
    }
  }
  var x_409 : bool;
  var x_456 : bool;
  var x_409_phi : bool;
  var x_412_phi : i32;
  var x_458_phi : bool;
  switch(0u) {
    default: {
      x_409_phi = false;
      x_412_phi = 0;
      loop {
        var x_437 : i32;
        var x_429 : i32;
        var x_410 : bool;
        var x_413 : i32;
        var x_410_phi : bool;
        var x_413_phi : i32;
        x_409 = x_409_phi;
        let x_412 : i32 = x_412_phi;
        x_458_phi = x_409;
        if ((x_412 <= 7)) {
        } else {
          break;
        }
        let x_418 : i32 = tree[x_412].data;
        if ((6 <= x_418)) {
          var x_456_phi : bool;
          let x_431 : ptr<function, i32> = &(tree[x_412].leftIndex);
          let x_432 : i32 = *(x_431);
          if ((x_432 == -1)) {
            let x_439 : f32 = x_8.injectionSwitch.y;
            let x_441 : f32 = x_8.injectionSwitch.x;
            if ((x_439 < x_441)) {
              loop {
                discard;
              }
              return;
            }
            *(x_431) = 7;
            tree[7] = BST(6, -1, -1);
            loop {
              x_456_phi = x_409;
              if ((0 < i32(x_439))) {
              } else {
                break;
              }
              x_456_phi = true;
              break;
            }
            x_456 = x_456_phi;
            x_458_phi = x_456;
            if (x_456) {
              break;
            }
          } else {
            x_437 = *(x_431);
            x_410_phi = x_409;
            x_413_phi = x_437;
            continue;
          }
        } else {
          let x_423 : ptr<function, i32> = &(tree[x_412].rightIndex);
          let x_424 : i32 = *(x_423);
          if ((x_424 == -1)) {
            *(x_423) = 7;
            tree[7] = BST(6, -1, -1);
            x_458_phi = true;
            break;
          } else {
            x_429 = *(x_423);
            x_410_phi = x_409;
            x_413_phi = x_429;
            continue;
          }
          return;
        }
        x_410_phi = x_456;
        x_413_phi = x_412;

        continuing {
          x_410 = x_410_phi;
          x_413 = x_413_phi;
          x_409_phi = x_410;
          x_412_phi = x_413;
        }
      }
      let x_458 : bool = x_458_phi;
      if (x_458) {
        break;
      }
    }
  }
  var x_463 : bool;
  var x_510 : bool;
  var x_463_phi : bool;
  var x_466_phi : i32;
  var x_512_phi : bool;
  switch(0u) {
    default: {
      x_463_phi = false;
      x_466_phi = 0;
      loop {
        var x_491 : i32;
        var x_483 : i32;
        var x_464 : bool;
        var x_467 : i32;
        var x_464_phi : bool;
        var x_467_phi : i32;
        x_463 = x_463_phi;
        let x_466 : i32 = x_466_phi;
        x_512_phi = x_463;
        if ((x_466 <= 8)) {
        } else {
          break;
        }
        let x_472 : i32 = tree[x_466].data;
        if ((17 <= x_472)) {
          var x_510_phi : bool;
          let x_485 : ptr<function, i32> = &(tree[x_466].leftIndex);
          let x_486 : i32 = *(x_485);
          if ((x_486 == -1)) {
            let x_493 : f32 = x_8.injectionSwitch.y;
            let x_495 : f32 = x_8.injectionSwitch.x;
            if ((x_493 < x_495)) {
              loop {
                discard;
              }
              return;
            }
            *(x_485) = 8;
            tree[8] = BST(17, -1, -1);
            loop {
              x_510_phi = x_463;
              if ((0 < i32(x_493))) {
              } else {
                break;
              }
              x_510_phi = true;
              break;
            }
            x_510 = x_510_phi;
            x_512_phi = x_510;
            if (x_510) {
              break;
            }
          } else {
            x_491 = *(x_485);
            x_464_phi = x_463;
            x_467_phi = x_491;
            continue;
          }
        } else {
          let x_477 : ptr<function, i32> = &(tree[x_466].rightIndex);
          let x_478 : i32 = *(x_477);
          if ((x_478 == -1)) {
            *(x_477) = 8;
            tree[8] = BST(17, -1, -1);
            x_512_phi = true;
            break;
          } else {
            x_483 = *(x_477);
            x_464_phi = x_463;
            x_467_phi = x_483;
            continue;
          }
          return;
        }
        x_464_phi = x_510;
        x_467_phi = x_466;

        continuing {
          x_464 = x_464_phi;
          x_467 = x_467_phi;
          x_463_phi = x_464;
          x_466_phi = x_467;
        }
      }
      let x_512 : bool = x_512_phi;
      if (x_512) {
        break;
      }
    }
  }
  var x_517 : bool;
  var x_564 : bool;
  var x_517_phi : bool;
  var x_520_phi : i32;
  var x_566_phi : bool;
  switch(0u) {
    default: {
      x_517_phi = false;
      x_520_phi = 0;
      loop {
        var x_545 : i32;
        var x_537 : i32;
        var x_518 : bool;
        var x_521 : i32;
        var x_518_phi : bool;
        var x_521_phi : i32;
        x_517 = x_517_phi;
        let x_520 : i32 = x_520_phi;
        x_566_phi = x_517;
        if ((x_520 <= 9)) {
        } else {
          break;
        }
        let x_526 : i32 = tree[x_520].data;
        if ((13 <= x_526)) {
          var x_564_phi : bool;
          let x_539 : ptr<function, i32> = &(tree[x_520].leftIndex);
          let x_540 : i32 = *(x_539);
          if ((x_540 == -1)) {
            let x_547 : f32 = x_8.injectionSwitch.y;
            let x_549 : f32 = x_8.injectionSwitch.x;
            if ((x_547 < x_549)) {
              loop {
                discard;
              }
              return;
            }
            *(x_539) = 9;
            tree[9] = BST(13, -1, -1);
            loop {
              x_564_phi = x_517;
              if ((0 < i32(x_547))) {
              } else {
                break;
              }
              x_564_phi = true;
              break;
            }
            x_564 = x_564_phi;
            x_566_phi = x_564;
            if (x_564) {
              break;
            }
          } else {
            x_545 = *(x_539);
            x_518_phi = x_517;
            x_521_phi = x_545;
            continue;
          }
        } else {
          let x_531 : ptr<function, i32> = &(tree[x_520].rightIndex);
          let x_532 : i32 = *(x_531);
          if ((x_532 == -1)) {
            *(x_531) = 9;
            tree[9] = BST(13, -1, -1);
            x_566_phi = true;
            break;
          } else {
            x_537 = *(x_531);
            x_518_phi = x_517;
            x_521_phi = x_537;
            continue;
          }
          return;
        }
        x_518_phi = x_564;
        x_521_phi = x_520;

        continuing {
          x_518 = x_518_phi;
          x_521 = x_521_phi;
          x_517_phi = x_518;
          x_520_phi = x_521;
        }
      }
      let x_566 : bool = x_566_phi;
      if (x_566) {
        break;
      }
    }
  }
  x_569_phi = 0;
  x_572_phi = 0;
  x_574_phi = 0;
  loop {
    var x_597 : i32;
    var x_607 : i32;
    var x_612 : i32;
    var x_575 : i32;
    var x_570_phi : i32;
    var x_573_phi : i32;
    let x_569 : i32 = x_569_phi;
    x_572 = x_572_phi;
    let x_574 : i32 = x_574_phi;
    if ((x_574 < 20)) {
    } else {
      break;
    }
    var x_582_phi : i32;
    var x_597_phi : i32;
    var x_598_phi : bool;
    switch(0u) {
      default: {
        x_582_phi = 0;
        loop {
          let x_582 : i32 = x_582_phi;
          x_597_phi = x_569;
          x_598_phi = false;
          if ((x_582 != -1)) {
          } else {
            break;
          }
          let x_589 : BST = tree[x_582];
          let x_590 : i32 = x_589.data;
          let x_591 : i32 = x_589.leftIndex;
          let x_592 : i32 = x_589.rightIndex;
          if ((x_590 == x_574)) {
            x_597_phi = x_574;
            x_598_phi = true;
            break;
          }

          continuing {
            x_582_phi = select(x_591, x_592, (x_574 > x_590));
          }
        }
        x_597 = x_597_phi;
        let x_598 : bool = x_598_phi;
        x_570_phi = x_597;
        if (x_598) {
          break;
        }
        x_570_phi = -1;
      }
    }
    var x_570 : i32;
    var x_606 : i32;
    var x_611 : i32;
    var x_607_phi : i32;
    var x_612_phi : i32;
    x_570 = x_570_phi;
    switch(x_574) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        x_607_phi = x_572;
        if ((x_570 == bitcast<i32>(x_574))) {
          x_606 = bitcast<i32>((x_572 + bitcast<i32>(1)));
          x_607_phi = x_606;
        }
        x_607 = x_607_phi;
        x_573_phi = x_607;
      }
      default: {
        x_612_phi = x_572;
        if ((x_570 == bitcast<i32>(-1))) {
          x_611 = bitcast<i32>((x_572 + bitcast<i32>(1)));
          x_612_phi = x_611;
        }
        x_612 = x_612_phi;
        x_573_phi = x_612;
      }
    }
    let x_573 : i32 = x_573_phi;

    continuing {
      x_575 = (x_574 + 1);
      x_569_phi = x_570;
      x_572_phi = x_573;
      x_574_phi = x_575;
    }
  }
  if ((x_572 == bitcast<i32>(20))) {
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
fn main([[builtin(position)]] gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
