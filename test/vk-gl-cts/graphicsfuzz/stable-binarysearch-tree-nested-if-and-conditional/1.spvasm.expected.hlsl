struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

static BST tree_1[10] = (BST[10])0;
cbuffer cbuffer_x_16 : register(b0, space0) {
  uint4 x_16[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  const int x_165 = data;
  tree.data = x_165;
  tree.leftIndex = -1;
  tree.rightIndex = -1;
  return;
}

void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = (BST)0;
  int param_1 = 0;
  int x_170 = 0;
  BST param_2 = (BST)0;
  int param_3 = 0;
  baseIndex = 0;
  while (true) {
    const int x_175 = baseIndex;
    const int x_176 = treeIndex;
    if ((x_175 <= x_176)) {
    } else {
      break;
    }
    const int x_179 = data_1;
    const int x_182 = tree_1[baseIndex].data;
    if ((x_179 <= x_182)) {
      const int x_189 = tree_1[baseIndex].leftIndex;
      if ((x_189 == -1)) {
        const int x_194 = baseIndex;
        const int x_195 = treeIndex;
        tree_1[x_194].leftIndex = x_195;
        const float x_198 = asfloat(x_16[0].x);
        const float x_200 = asfloat(x_16[0].y);
        if ((x_198 < x_200)) {
          const int x_204 = treeIndex;
          const BST x_206 = tree_1[x_204];
          param = x_206;
          const int x_207 = data_1;
          param_1 = x_207;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
          tree_1[x_204] = param;
        }
        const float x_212 = asfloat(x_16[0].x);
        const float x_214 = asfloat(x_16[0].y);
        if ((x_212 < x_214)) {
          return;
        }
      } else {
        const int x_220 = tree_1[baseIndex].leftIndex;
        baseIndex = x_220;
        continue;
      }
    } else {
      const float x_222 = asfloat(x_16[0].x);
      const float x_224 = asfloat(x_16[0].y);
      if ((x_222 < x_224)) {
        const int x_231 = tree_1[baseIndex].rightIndex;
        x_170 = x_231;
      } else {
        const int x_234 = tree_1[baseIndex].rightIndex;
        x_170 = x_234;
      }
      if ((x_170 == -1)) {
        const int x_240 = baseIndex;
        const int x_241 = treeIndex;
        tree_1[x_240].rightIndex = x_241;
        const int x_243 = treeIndex;
        const BST x_245 = tree_1[x_243];
        param_2 = x_245;
        const int x_246 = data_1;
        param_3 = x_246;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
        tree_1[x_243] = param_2;
        return;
      } else {
        const int x_252 = tree_1[baseIndex].rightIndex;
        baseIndex = x_252;
        continue;
      }
      return;
    }
    const float x_254 = asfloat(x_16[0].x);
    const float x_256 = asfloat(x_16[0].y);
    if ((x_254 > x_256)) {
      return;
    }
  }
  return;
}

int search_i1_(inout int target) {
  int index = 0;
  BST currentNode = (BST)0;
  int x_261 = 0;
  index = 0;
  while (true) {
    if ((index != -1)) {
    } else {
      break;
    }
    const BST x_271 = tree_1[index];
    currentNode = x_271;
    const int x_273 = currentNode.data;
    const int x_274 = target;
    if ((x_273 == x_274)) {
      const int x_278 = target;
      return x_278;
    }
    const int x_279 = target;
    const int x_281 = currentNode.data;
    if ((x_279 > x_281)) {
      const int x_287 = currentNode.rightIndex;
      x_261 = x_287;
    } else {
      const int x_289 = currentNode.leftIndex;
      x_261 = x_289;
    }
    index = x_261;
  }
  return -1;
}

void main_1() {
  int treeIndex_1 = 0;
  BST param_4 = (BST)0;
  int param_5 = 0;
  int param_6 = 0;
  int param_7 = 0;
  int param_8 = 0;
  int param_9 = 0;
  int param_10 = 0;
  int param_11 = 0;
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
  int count = 0;
  int i = 0;
  int result = 0;
  int param_24 = 0;
  treeIndex_1 = 0;
  const BST x_91 = tree_1[0];
  param_4 = x_91;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  tree_1[0] = param_4;
  treeIndex_1 = (treeIndex_1 + 1);
  param_6 = treeIndex_1;
  param_7 = 5;
  insert_i1_i1_(param_6, param_7);
  treeIndex_1 = (treeIndex_1 + 1);
  param_8 = treeIndex_1;
  param_9 = 12;
  insert_i1_i1_(param_8, param_9);
  treeIndex_1 = (treeIndex_1 + 1);
  param_10 = treeIndex_1;
  param_11 = 15;
  insert_i1_i1_(param_10, param_11);
  treeIndex_1 = (treeIndex_1 + 1);
  param_12 = treeIndex_1;
  param_13 = 7;
  insert_i1_i1_(param_12, param_13);
  treeIndex_1 = (treeIndex_1 + 1);
  param_14 = treeIndex_1;
  param_15 = 8;
  insert_i1_i1_(param_14, param_15);
  treeIndex_1 = (treeIndex_1 + 1);
  param_16 = treeIndex_1;
  param_17 = 2;
  insert_i1_i1_(param_16, param_17);
  treeIndex_1 = (treeIndex_1 + 1);
  param_18 = treeIndex_1;
  param_19 = 6;
  insert_i1_i1_(param_18, param_19);
  treeIndex_1 = (treeIndex_1 + 1);
  param_20 = treeIndex_1;
  param_21 = 17;
  insert_i1_i1_(param_20, param_21);
  treeIndex_1 = (treeIndex_1 + 1);
  param_22 = treeIndex_1;
  param_23 = 13;
  insert_i1_i1_(param_22, param_23);
  count = 0;
  i = 0;
  {
    for(; (i < 20); i = (i + 1)) {
      param_24 = i;
      const int x_139 = search_i1_(param_24);
      result = x_139;
      switch(i) {
        case 2:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 12:
        case 13:
        case 15:
        case 17: {
          if ((result == i)) {
            count = (count + 1);
          }
          break;
        }
        default: {
          if ((result == -1)) {
            count = (count + 1);
          }
          break;
        }
      }
    }
  }
  if ((count == 20)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 1.0f, 1.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
