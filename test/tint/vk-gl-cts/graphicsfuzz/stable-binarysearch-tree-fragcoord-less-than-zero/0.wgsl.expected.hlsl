SKIP: FAILED https://crbug.com/tint/1522

vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-fragcoord-less-than-zero/0.wgsl:58:7 warning: code is unreachable
      return;
      ^^^^^^

vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-fragcoord-less-than-zero/0.wgsl:81:7 warning: code is unreachable
      return;
      ^^^^^^

vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-fragcoord-less-than-zero/0.wgsl:83:5 warning: code is unreachable
    return;
    ^^^^^^

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

static BST tree_1[10] = (BST[10])0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  const int x_158 = data;
  tree.data = x_158;
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
    const int x_167 = baseIndex;
    const int x_168 = treeIndex;
    if ((x_167 <= x_168)) {
    } else {
      break;
    }
    const int x_171 = data_1;
    const int x_174 = tree_1[baseIndex].data;
    if ((x_171 <= x_174)) {
      const int x_181 = tree_1[baseIndex].leftIndex;
      if ((x_181 == -1)) {
        const int x_186 = baseIndex;
        const int x_187 = treeIndex;
        tree_1[x_186].leftIndex = x_187;
        const int x_189 = treeIndex;
        const BST x_191 = tree_1[x_189];
        param = x_191;
        const int x_192 = data_1;
        param_1 = x_192;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
        tree_1[x_189] = param;
        return;
      } else {
        const int x_198 = tree_1[baseIndex].leftIndex;
        baseIndex = x_198;
        continue;
      }
      return;
    } else {
      const int x_201 = tree_1[baseIndex].rightIndex;
      if ((x_201 == -1)) {
        const int x_206 = baseIndex;
        const int x_207 = treeIndex;
        tree_1[x_206].rightIndex = x_207;
        const int x_209 = treeIndex;
        const BST x_211 = tree_1[x_209];
        param_2 = x_211;
        const int x_212 = data_1;
        param_3 = x_212;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
        tree_1[x_209] = param_2;
        return;
      } else {
        const int x_218 = tree_1[baseIndex].rightIndex;
        baseIndex = x_218;
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
  int x_220 = 0;
  index = 0;
  [loop] while (true) {
    if ((index != -1)) {
    } else {
      break;
    }
    const BST x_230 = tree_1[index];
    currentNode = x_230;
    const int x_232 = currentNode.data;
    const int x_233 = target;
    if ((x_232 == x_233)) {
      const int x_237 = target;
      return x_237;
    }
    const int x_238 = target;
    const int x_240 = currentNode.data;
    if ((x_238 > x_240)) {
      const int x_246 = currentNode.rightIndex;
      x_220 = x_246;
    } else {
      const int x_248 = currentNode.leftIndex;
      x_220 = x_248;
    }
    index = x_220;
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
  const BST x_84 = tree_1[0];
  param_4 = x_84;
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
      param_24 = i;
      const int x_132 = search_i1_(param_24);
      result = x_132;
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x000001CA0D820F20(25,10-21): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
internal error: compilation aborted unexpectedly

