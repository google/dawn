SKIP: FAILED https://crbug.com/tint/1522

warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

static BST tree_1[10] = (BST[10])0;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  const int x_169 = data;
  tree.data = x_169;
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
  baseIndex = 0;
  [loop] while (true) {
    const int x_178 = baseIndex;
    const int x_179 = treeIndex;
    if ((x_178 <= x_179)) {
    } else {
      break;
    }
    const int x_182 = data_1;
    const int x_185 = tree_1[baseIndex].data;
    if ((x_182 <= x_185)) {
      const int x_192 = tree_1[baseIndex].leftIndex;
      if ((x_192 == -1)) {
        const int x_197 = baseIndex;
        const int x_198 = treeIndex;
        tree_1[x_197].leftIndex = x_198;
        const int x_200 = treeIndex;
        const BST x_202 = tree_1[x_200];
        param = x_202;
        const int x_203 = data_1;
        param_1 = x_203;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
        tree_1[x_200] = param;
        return;
      } else {
        const int x_209 = tree_1[baseIndex].leftIndex;
        baseIndex = x_209;
        continue;
      }
      return;
    } else {
      const int x_212 = tree_1[baseIndex].rightIndex;
      if ((x_212 == -1)) {
        const int x_217 = baseIndex;
        const int x_218 = treeIndex;
        tree_1[x_217].rightIndex = x_218;
        const int x_220 = treeIndex;
        const BST x_222 = tree_1[x_220];
        param_2 = x_222;
        const int x_223 = data_1;
        param_3 = x_223;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
        tree_1[x_220] = param_2;
        return;
      } else {
        const int x_229 = tree_1[baseIndex].rightIndex;
        baseIndex = x_229;
        continue;
      }
      return;
    }
    return;
  }
  return;
}

int search_i1_(inout int target) {
  int index = 0;
  BST currentNode = (BST)0;
  int x_231 = 0;
  index = 0;
  [loop] while (true) {
    if ((index != -1)) {
    } else {
      break;
    }
    const BST x_241 = tree_1[index];
    currentNode = x_241;
    const int x_243 = currentNode.data;
    const int x_244 = target;
    if ((x_243 == x_244)) {
      const int x_248 = target;
      return x_248;
    }
    const int x_249 = target;
    const int x_251 = currentNode.data;
    if ((x_249 > x_251)) {
      const int x_257 = currentNode.rightIndex;
      x_231 = x_257;
    } else {
      const int x_259 = currentNode.leftIndex;
      x_231 = x_259;
    }
    index = x_231;
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
  const BST x_88 = tree_1[0];
  param_4 = x_88;
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
    [loop] for(; (i < 20); i = (i + 1)) {
      bool x_155 = false;
      bool x_156_phi = false;
      param_24 = i;
      const int x_136 = search_i1_(param_24);
      result = x_136;
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
          const bool x_149 = (result == i);
          x_156_phi = x_149;
          if (!(x_149)) {
            const float x_154 = gl_FragCoord.x;
            x_155 = (x_154 < 0.0f);
            x_156_phi = x_155;
          }
          if (x_156_phi) {
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
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x000002A8C1306820(26,10-21): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
internal error: compilation aborted unexpectedly

