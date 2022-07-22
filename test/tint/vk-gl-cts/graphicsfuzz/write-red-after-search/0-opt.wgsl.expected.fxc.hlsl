SKIP: FAILED https://crbug.com/tint/1522

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};
struct Obj {
  float odd_numbers[10];
  float even_numbers[10];
};

static BST tree_1[10] = (BST[10])0;
cbuffer cbuffer_x_27 : register(b0, space0) {
  uint4 x_27[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  const int x_74 = data;
  tree.data = x_74;
  tree.leftIndex = -1;
  tree.rightIndex = -1;
  return;
}

void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = (BST)0;
  int param_1 = 0;
  BST param_2 = (BST)0;
  int param_3 = 0;
  int GLF_live8i = 0;
  float GLF_live8A[50] = (float[50])0;
  baseIndex = 0;
  [loop] while (true) {
    const int x_75 = baseIndex;
    const int x_76 = treeIndex;
    if ((x_75 <= x_76)) {
    } else {
      break;
    }
    const int x_77 = data_1;
    const int x_79 = tree_1[baseIndex].data;
    if ((x_77 <= x_79)) {
      const int x_81 = tree_1[baseIndex].leftIndex;
      if ((x_81 == -1)) {
        const int x_82 = baseIndex;
        const int x_83 = treeIndex;
        tree_1[x_82].leftIndex = x_83;
        const int x_84 = treeIndex;
        const BST x_350 = tree_1[x_84];
        param = x_350;
        const int x_85 = data_1;
        param_1 = x_85;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
        tree_1[x_84] = param;
        return;
      } else {
        const int x_87 = tree_1[baseIndex].leftIndex;
        baseIndex = x_87;
        continue;
      }
    } else {
      const int x_89 = tree_1[baseIndex].rightIndex;
      if ((x_89 == -1)) {
        const int x_90 = baseIndex;
        const int x_91 = treeIndex;
        tree_1[x_90].rightIndex = x_91;
        const int x_92 = treeIndex;
        const BST x_362 = tree_1[x_92];
        param_2 = x_362;
        const int x_93 = data_1;
        param_3 = x_93;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
        tree_1[x_92] = param_2;
        return;
      } else {
        GLF_live8i = 1;
        bool tint_tmp = (GLF_live8i >= 0);
        if (tint_tmp) {
          tint_tmp = (GLF_live8i < 50);
        }
        const int x_369 = ((tint_tmp) ? GLF_live8i : 0);
        const float x_371 = GLF_live8A[0];
        const float x_373 = GLF_live8A[x_369];
        GLF_live8A[x_369] = (x_373 + x_371);
        [loop] while (true) {
          const int x_98 = tree_1[baseIndex].rightIndex;
          baseIndex = x_98;
          {
            const float x_382 = asfloat(x_27[0].x);
            const float x_384 = asfloat(x_27[0].y);
            if ((x_382 > x_384)) {
            } else {
              break;
            }
          }
        }
        continue;
      }
    }
  }
  return;
}

int search_i1_(inout int target) {
  int index = 0;
  BST currentNode = (BST)0;
  int x_387 = 0;
  index = 0;
  [loop] while (true) {
    if ((index != -1)) {
    } else {
      break;
    }
    const BST x_395 = tree_1[index];
    currentNode = x_395;
    const int x_101 = currentNode.data;
    const int x_102 = target;
    if ((x_101 == x_102)) {
      const int x_103 = target;
      return x_103;
    }
    const int x_104 = target;
    const int x_105 = currentNode.data;
    if ((x_104 > x_105)) {
      const int x_106 = currentNode.rightIndex;
      x_387 = x_106;
    } else {
      const int x_107 = currentNode.leftIndex;
      x_387 = x_107;
    }
    index = x_387;
  }
  return -1;
}

float makeFrame_f1_(inout float v) {
  int param_5 = 0;
  int param_6 = 0;
  int param_7 = 0;
  const float x_418 = v;
  v = (x_418 * 6.5f);
  const float x_420 = v;
  if ((x_420 < 1.5f)) {
    param_5 = 100;
    const int x_110 = search_i1_(param_5);
    return float(x_110);
  }
  const float x_425 = v;
  if ((x_425 < 4.0f)) {
    return 0.0f;
  }
  const float x_429 = v;
  param_6 = 6;
  const int x_111 = search_i1_(param_6);
  if ((x_429 < float(x_111))) {
    return 1.0f;
  }
  param_7 = 30;
  const int x_112 = search_i1_(param_7);
  return (10.0f + float(x_112));
}

float3 hueColor_f1_(inout float angle) {
  float nodeData = 0.0f;
  int param_4 = 0;
  param_4 = 15;
  const int x_109 = search_i1_(param_4);
  nodeData = float(x_109);
  const float x_409 = angle;
  return ((float3(30.0f, 30.0f, 30.0f) + (float3(1.0f, 5.0f, nodeData) * x_409)) / float3(50.0f, 50.0f, 50.0f));
}

void main_1() {
  int treeIndex_1 = 0;
  BST param_8 = (BST)0;
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
  Obj GLF_live4obj = (Obj)0;
  int param_24 = 0;
  int param_25 = 0;
  int param_26 = 0;
  int param_27 = 0;
  float2 z = float2(0.0f, 0.0f);
  float x_1 = 0.0f;
  float param_28 = 0.0f;
  float y_1 = 0.0f;
  float param_29 = 0.0f;
  int sum = 0;
  int target_1 = 0;
  int result = 0;
  int param_30 = 0;
  float a = 0.0f;
  float3 x_235 = float3(0.0f, 0.0f, 0.0f);
  float param_31 = 0.0f;
  treeIndex_1 = 0;
  const BST x_237 = tree_1[0];
  param_8 = x_237;
  param_9 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_8, param_9);
  tree_1[0] = param_8;
  treeIndex_1 = (treeIndex_1 + 1);
  param_10 = treeIndex_1;
  param_11 = 5;
  insert_i1_i1_(param_10, param_11);
  treeIndex_1 = (treeIndex_1 + 1);
  GLF_live1_looplimiter2 = 0;
  GLF_live1i = 0;
  {
    [loop] for(; true; GLF_live1i = (GLF_live1i + 1)) {
      if ((GLF_live1_looplimiter2 >= 7)) {
        break;
      }
      GLF_live1_looplimiter2 = (GLF_live1_looplimiter2 + 1);
    }
  }
  param_12 = treeIndex_1;
  param_13 = 12;
  insert_i1_i1_(param_12, param_13);
  treeIndex_1 = (treeIndex_1 + 1);
  param_14 = treeIndex_1;
  param_15 = 15;
  insert_i1_i1_(param_14, param_15);
  treeIndex_1 = (treeIndex_1 + 1);
  param_16 = treeIndex_1;
  param_17 = 7;
  insert_i1_i1_(param_16, param_17);
  treeIndex_1 = (treeIndex_1 + 1);
  param_18 = treeIndex_1;
  param_19 = 8;
  insert_i1_i1_(param_18, param_19);
  treeIndex_1 = (treeIndex_1 + 1);
  param_20 = treeIndex_1;
  param_21 = 2;
  insert_i1_i1_(param_20, param_21);
  treeIndex_1 = (treeIndex_1 + 1);
  param_22 = treeIndex_1;
  param_23 = 6;
  insert_i1_i1_(param_22, param_23);
  treeIndex_1 = (treeIndex_1 + 1);
  GLF_live4_looplimiter3 = 0;
  GLF_live4i = 0;
  {
    [loop] for(; true; GLF_live4i = (GLF_live4i + 1)) {
      if ((GLF_live4_looplimiter3 >= 3)) {
        break;
      }
      GLF_live4_looplimiter3 = (GLF_live4_looplimiter3 + 1);
      GLF_live4index = 1;
      const int x_144 = GLF_live4index;
      const int x_145 = GLF_live4index;
      const int x_146 = GLF_live4index;
      const float x_269 = GLF_live4obj.even_numbers[1];
      {
        float tint_symbol_1[10] = GLF_live4obj.even_numbers;
        bool tint_tmp_1 = (x_144 >= 0);
        if (tint_tmp_1) {
          tint_tmp_1 = (x_145 < 10);
        }
        tint_symbol_1[((tint_tmp_1) ? x_146 : 0)] = x_269;
        GLF_live4obj.even_numbers = tint_symbol_1;
      }
      const int x_147 = GLF_live4i;
      const int x_148 = GLF_live4i;
      const int x_149 = GLF_live4i;
      {
        float tint_symbol_3[10] = GLF_live4obj.even_numbers;
        bool tint_tmp_2 = (x_147 >= 0);
        if (tint_tmp_2) {
          tint_tmp_2 = (x_148 < 10);
        }
        tint_symbol_3[((tint_tmp_2) ? x_149 : 0)] = 1.0f;
        GLF_live4obj.even_numbers = tint_symbol_3;
      }
    }
  }
  param_24 = treeIndex_1;
  param_25 = 17;
  insert_i1_i1_(param_24, param_25);
  const float x_278 = asfloat(x_27[0].x);
  const float x_280 = asfloat(x_27[0].y);
  if ((x_278 > x_280)) {
    return;
  }
  treeIndex_1 = (treeIndex_1 + 1);
  param_26 = treeIndex_1;
  param_27 = 13;
  insert_i1_i1_(param_26, param_27);
  const float4 x_285 = gl_FragCoord;
  z = (float2(x_285.y, x_285.x) / float2(256.0f, 256.0f));
  const float x_289 = z.x;
  param_28 = x_289;
  const float x_290 = makeFrame_f1_(param_28);
  x_1 = x_290;
  const float x_292 = z.y;
  param_29 = x_292;
  const float x_293 = makeFrame_f1_(param_29);
  y_1 = x_293;
  sum = -100;
  target_1 = 0;
  {
    [loop] for(; (target_1 < 20); target_1 = (target_1 + 1)) {
      param_30 = target_1;
      const int x_158 = search_i1_(param_30);
      result = x_158;
      if ((result > 0)) {
      } else {
        switch(result) {
          case 0: {
            return;
            break;
          }
          case -1: {
            sum = (sum + 1);
            break;
          }
          default: {
            break;
          }
        }
      }
    }
  }
  a = (x_1 + (y_1 * float(sum)));
  const float x_313 = asfloat(x_27[0].x);
  const float x_315 = asfloat(x_27[0].y);
  if ((x_313 < x_315)) {
    x_235 = float3(1.0f, 0.0f, 0.0f);
  } else {
    param_31 = a;
    const float3 x_321 = hueColor_f1_(param_31);
    x_235 = x_321;
  }
  const float3 x_322 = x_235;
  x_GLF_color = float4(x_322.x, x_322.y, x_322.z, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_5 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_6 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_8 = {x_GLF_color};
  return tint_symbol_8;
}

tint_symbol_6 main(tint_symbol_5 tint_symbol_4) {
  const main_out inner_result = main_inner(tint_symbol_4.gl_FragCoord_param);
  tint_symbol_6 wrapper_result = (tint_symbol_6)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x000001C9889DF080(35,10-21): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\tint\test\Shader@0x000001C9889DF080(149,3): warning X4000: use of potentially uninitialized variable (makeFrame_f1_)
internal error: compilation aborted unexpectedly

