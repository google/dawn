SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

struct buf0 {
  vec2 injectionSwitch;
};

struct Obj {
  float odd_numbers[10];
  float even_numbers[10];
};

struct main_out {
  vec4 x_GLF_color_1;
};

BST tree_1[10] = BST[10](BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0));
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  int x_74 = data;
  tree.data = x_74;
  tree.leftIndex = -1;
  tree.rightIndex = -1;
}
void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = BST(0, 0, 0);
  int param_1 = 0;
  BST param_2 = BST(0, 0, 0);
  int param_3 = 0;
  int GLF_live8i = 0;
  float GLF_live8A[50] = float[50](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  baseIndex = 0;
  {
    while(true) {
      int x_75 = baseIndex;
      int x_76 = treeIndex;
      if ((x_75 <= x_76)) {
      } else {
        break;
      }
      int x_77 = data_1;
      int x_78 = baseIndex;
      int x_79 = tree_1[x_78].data;
      if ((x_77 <= x_79)) {
        int x_80 = baseIndex;
        int x_81 = tree_1[x_80].leftIndex;
        if ((x_81 == -1)) {
          int x_82 = baseIndex;
          int x_83 = treeIndex;
          tree_1[x_82].leftIndex = x_83;
          int x_84 = treeIndex;
          BST x_350 = tree_1[x_84];
          param = x_350;
          int x_85 = data_1;
          param_1 = x_85;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
          BST x_352 = param;
          tree_1[x_84] = x_352;
          return;
        } else {
          int x_86 = baseIndex;
          int x_87 = tree_1[x_86].leftIndex;
          baseIndex = x_87;
          {
          }
          continue;
        }
      } else {
        int x_88 = baseIndex;
        int x_89 = tree_1[x_88].rightIndex;
        if ((x_89 == -1)) {
          int x_90 = baseIndex;
          int x_91 = treeIndex;
          tree_1[x_90].rightIndex = x_91;
          int x_92 = treeIndex;
          BST x_362 = tree_1[x_92];
          param_2 = x_362;
          int x_93 = data_1;
          param_3 = x_93;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
          BST x_364 = param_2;
          tree_1[x_92] = x_364;
          return;
        } else {
          GLF_live8i = 1;
          int x_94 = GLF_live8i;
          int x_95 = GLF_live8i;
          int x_96 = GLF_live8i;
          int x_369 = ((((x_94 >= 0) & (x_95 < 50))) ? (x_96) : (0));
          float x_371 = GLF_live8A[0];
          float x_373 = GLF_live8A[x_369];
          GLF_live8A[x_369] = (x_373 + x_371);
          {
            while(true) {
              int x_97 = baseIndex;
              int x_98 = tree_1[x_97].rightIndex;
              baseIndex = x_98;
              {
                float x_382 = v_1.tint_symbol_3.injectionSwitch.x;
                float x_384 = v_1.tint_symbol_3.injectionSwitch.y;
                if (!((x_382 > x_384))) { break; }
              }
              continue;
            }
          }
          {
          }
          continue;
        }
      }
      /* unreachable */
    }
  }
}
int search_i1_(inout int t) {
  int index = 0;
  BST currentNode = BST(0, 0, 0);
  int x_387 = 0;
  index = 0;
  {
    while(true) {
      int x_99 = index;
      if ((x_99 != -1)) {
      } else {
        break;
      }
      int x_100 = index;
      BST x_395 = tree_1[x_100];
      currentNode = x_395;
      int x_101 = currentNode.data;
      int x_102 = t;
      if ((x_101 == x_102)) {
        int x_103 = t;
        return x_103;
      }
      int x_104 = t;
      int x_105 = currentNode.data;
      if ((x_104 > x_105)) {
        int x_106 = currentNode.rightIndex;
        x_387 = x_106;
      } else {
        int x_107 = currentNode.leftIndex;
        x_387 = x_107;
      }
      int x_108 = x_387;
      index = x_108;
      {
      }
      continue;
    }
  }
  return -1;
}
float makeFrame_f1_(inout float v) {
  int param_5 = 0;
  int param_6 = 0;
  int param_7 = 0;
  float x_418 = v;
  v = (x_418 * 6.5f);
  float x_420 = v;
  if ((x_420 < 1.5f)) {
    param_5 = 100;
    int x_110 = search_i1_(param_5);
    return float(x_110);
  }
  float x_425 = v;
  if ((x_425 < 4.0f)) {
    return 0.0f;
  }
  float x_429 = v;
  param_6 = 6;
  int x_111 = search_i1_(param_6);
  if ((x_429 < float(x_111))) {
    return 1.0f;
  }
  param_7 = 30;
  int x_112 = search_i1_(param_7);
  return (10.0f + float(x_112));
}
vec3 hueColor_f1_(inout float angle) {
  float nodeData = 0.0f;
  int param_4 = 0;
  param_4 = 15;
  int x_109 = search_i1_(param_4);
  nodeData = float(x_109);
  float x_409 = angle;
  float x_410 = nodeData;
  return ((vec3(30.0f) + (vec3(1.0f, 5.0f, x_410) * x_409)) / vec3(50.0f));
}
void main_1() {
  int treeIndex_1 = 0;
  BST param_8 = BST(0, 0, 0);
  int param_9 = 0;
  int param_10 = 0;
  int param_11 = 0;
  int GLF_live1_looplimiter2 = 0;
  int GLF_live1i = 0;
  int param_12 = 0;
  int param_13 = 0;
  int param_14 = 0;
  int param_15 = 0;
  int param_16 = 0;
  int param_17 = 0;
  int param_18 = 0;
  int param_19 = 0;
  int param_20 = 0;
  int param_21 = 0;
  int param_22 = 0;
  int param_23 = 0;
  int GLF_live4_looplimiter3 = 0;
  int GLF_live4i = 0;
  int GLF_live4index = 0;
  Obj GLF_live4obj = Obj(float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  int param_24 = 0;
  int param_25 = 0;
  int param_26 = 0;
  int param_27 = 0;
  vec2 z = vec2(0.0f);
  float x = 0.0f;
  float param_28 = 0.0f;
  float y = 0.0f;
  float param_29 = 0.0f;
  int sum = 0;
  int t_1 = 0;
  int result = 0;
  int param_30 = 0;
  float a = 0.0f;
  vec3 x_235 = vec3(0.0f);
  float param_31 = 0.0f;
  treeIndex_1 = 0;
  BST x_237 = tree_1[0];
  param_8 = x_237;
  param_9 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_8, param_9);
  BST x_239 = param_8;
  tree_1[0] = x_239;
  int x_113 = treeIndex_1;
  treeIndex_1 = (x_113 + 1);
  int x_115 = treeIndex_1;
  param_10 = x_115;
  param_11 = 5;
  insert_i1_i1_(param_10, param_11);
  int x_116 = treeIndex_1;
  treeIndex_1 = (x_116 + 1);
  GLF_live1_looplimiter2 = 0;
  GLF_live1i = 0;
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      int x_118 = GLF_live1_looplimiter2;
      if ((x_118 >= 7)) {
        break;
      }
      int x_119 = GLF_live1_looplimiter2;
      GLF_live1_looplimiter2 = (x_119 + 1);
      {
        int x_121 = GLF_live1i;
        GLF_live1i = (x_121 + 1);
      }
      continue;
    }
  }
  int x_123 = treeIndex_1;
  param_12 = x_123;
  param_13 = 12;
  insert_i1_i1_(param_12, param_13);
  int x_124 = treeIndex_1;
  treeIndex_1 = (x_124 + 1);
  int x_126 = treeIndex_1;
  param_14 = x_126;
  param_15 = 15;
  insert_i1_i1_(param_14, param_15);
  int x_127 = treeIndex_1;
  treeIndex_1 = (x_127 + 1);
  int x_129 = treeIndex_1;
  param_16 = x_129;
  param_17 = 7;
  insert_i1_i1_(param_16, param_17);
  int x_130 = treeIndex_1;
  treeIndex_1 = (x_130 + 1);
  int x_132 = treeIndex_1;
  param_18 = x_132;
  param_19 = 8;
  insert_i1_i1_(param_18, param_19);
  int x_133 = treeIndex_1;
  treeIndex_1 = (x_133 + 1);
  int x_135 = treeIndex_1;
  param_20 = x_135;
  param_21 = 2;
  insert_i1_i1_(param_20, param_21);
  int x_136 = treeIndex_1;
  treeIndex_1 = (x_136 + 1);
  int x_138 = treeIndex_1;
  param_22 = x_138;
  param_23 = 6;
  insert_i1_i1_(param_22, param_23);
  int x_139 = treeIndex_1;
  treeIndex_1 = (x_139 + 1);
  GLF_live4_looplimiter3 = 0;
  GLF_live4i = 0;
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      int x_141 = GLF_live4_looplimiter3;
      if ((x_141 >= 3)) {
        break;
      }
      int x_142 = GLF_live4_looplimiter3;
      GLF_live4_looplimiter3 = (x_142 + 1);
      GLF_live4index = 1;
      int x_144 = GLF_live4index;
      int x_145 = GLF_live4index;
      int x_146 = GLF_live4index;
      float x_269 = GLF_live4obj.even_numbers[1];
      GLF_live4obj.even_numbers[((((x_144 >= 0) & (x_145 < 10))) ? (x_146) : (0))] = x_269;
      int x_147 = GLF_live4i;
      int x_148 = GLF_live4i;
      int x_149 = GLF_live4i;
      GLF_live4obj.even_numbers[((((x_147 >= 0) & (x_148 < 10))) ? (x_149) : (0))] = 1.0f;
      {
        int x_150 = GLF_live4i;
        GLF_live4i = (x_150 + 1);
      }
      continue;
    }
  }
  int x_152 = treeIndex_1;
  param_24 = x_152;
  param_25 = 17;
  insert_i1_i1_(param_24, param_25);
  float x_278 = v_1.tint_symbol_3.injectionSwitch.x;
  float x_280 = v_1.tint_symbol_3.injectionSwitch.y;
  if ((x_278 > x_280)) {
    return;
  }
  int x_153 = treeIndex_1;
  treeIndex_1 = (x_153 + 1);
  int x_155 = treeIndex_1;
  param_26 = x_155;
  param_27 = 13;
  insert_i1_i1_(param_26, param_27);
  vec4 x_285 = tint_symbol;
  z = (vec2(x_285[1u], x_285[0u]) / vec2(256.0f));
  float x_289 = z.x;
  param_28 = x_289;
  float x_290 = makeFrame_f1_(param_28);
  x = x_290;
  float x_292 = z.y;
  param_29 = x_292;
  float x_293 = makeFrame_f1_(param_29);
  y = x_293;
  sum = -100;
  t_1 = 0;
  {
    while(true) {
      int x_156 = t_1;
      if ((x_156 < 20)) {
      } else {
        break;
      }
      int x_157 = t_1;
      param_30 = x_157;
      int x_158 = search_i1_(param_30);
      result = x_158;
      int x_159 = result;
      if ((x_159 > 0)) {
      } else {
        int x_160 = result;
        switch(x_160) {
          case 0:
          {
            return;
          }
          case -1:
          {
            int x_161 = sum;
            sum = (x_161 + 1);
            break;
          }
          default:
          {
            break;
          }
        }
      }
      {
        int x_163 = t_1;
        t_1 = (x_163 + 1);
      }
      continue;
    }
  }
  float x_307 = x;
  float x_308 = y;
  int x_165 = sum;
  a = (x_307 + (x_308 * float(x_165)));
  float x_313 = v_1.tint_symbol_3.injectionSwitch.x;
  float x_315 = v_1.tint_symbol_3.injectionSwitch.y;
  if ((x_313 < x_315)) {
    x_235 = vec3(1.0f, 0.0f, 0.0f);
  } else {
    float x_320 = a;
    param_31 = x_320;
    vec3 x_321 = hueColor_f1_(param_31);
    x_235 = x_321;
  }
  vec3 x_322 = x_235;
  x_GLF_color = vec4(x_322[0u], x_322[1u], x_322[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:104: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:104: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
