SKIP: FAILED https://crbug.com/tint/1522

warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};
struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static BST tree[10] = (BST[10])0;
cbuffer cbuffer_x_50 : register(b0, space0) {
  uint4 x_50[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST node, inout int data) {
  const int x_208 = data;
  node.data = x_208;
  node.leftIndex = -1;
  node.rightIndex = -1;
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
    const int x_217 = baseIndex;
    const int x_218 = treeIndex;
    if ((x_217 <= x_218)) {
    } else {
      break;
    }
    const int x_221 = data_1;
    const int x_224 = tree[baseIndex].data;
    if ((x_221 <= x_224)) {
      const int x_231 = tree[baseIndex].leftIndex;
      if ((x_231 == -1)) {
        const int x_236 = baseIndex;
        const int x_237 = treeIndex;
        tree[x_236].leftIndex = x_237;
        const int x_239 = treeIndex;
        const BST x_241 = tree[x_239];
        param = x_241;
        const int x_242 = data_1;
        param_1 = x_242;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
        tree[x_239] = param;
        return;
      } else {
        const int x_248 = tree[baseIndex].leftIndex;
        baseIndex = x_248;
        continue;
      }
      return;
    } else {
      const int x_251 = tree[baseIndex].rightIndex;
      if ((x_251 == -1)) {
        const int x_256 = baseIndex;
        const int x_257 = treeIndex;
        tree[x_256].rightIndex = x_257;
        const int x_259 = treeIndex;
        const BST x_261 = tree[x_259];
        param_2 = x_261;
        const int x_262 = data_1;
        param_3 = x_262;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
        tree[x_259] = param_2;
        return;
      } else {
        const int x_268 = tree[baseIndex].rightIndex;
        baseIndex = x_268;
        continue;
      }
      return;
    }
    return;
  }
  return;
}

int identity_i1_(inout int a) {
  const int x_202 = a;
  const int x_203 = a;
  {
    int tint_symbol_1[10] = obj.numbers;
    tint_symbol_1[x_202] = x_203;
    obj.numbers = tint_symbol_1;
  }
  const int x_206 = obj.numbers[2];
  return x_206;
}

int search_i1_(inout int target) {
  int index = 0;
  BST currentNode = (BST)0;
  int x_270 = 0;
  index = 0;
  [loop] while (true) {
    if ((index != -1)) {
    } else {
      break;
    }
    const BST x_280 = tree[index];
    currentNode = x_280;
    const int x_282 = currentNode.data;
    const int x_283 = target;
    if ((x_282 == x_283)) {
      const int x_287 = target;
      return x_287;
    }
    const int x_288 = target;
    const int x_290 = currentNode.data;
    if ((x_288 > x_290)) {
      const int x_296 = currentNode.rightIndex;
      x_270 = x_296;
    } else {
      const int x_298 = currentNode.leftIndex;
      x_270 = x_298;
    }
    index = x_270;
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
  int pp = 0;
  int looplimiter0 = 0;
  int i = 0;
  int param_24 = 0;
  int count = 0;
  int i_1 = 0;
  int result = 0;
  int param_25 = 0;
  treeIndex_1 = 0;
  const BST x_101 = tree[0];
  param_4 = x_101;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  tree[0] = param_4;
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
  pp = 0;
  looplimiter0 = 0;
  i = 0;
  {
    [loop] for(; (i < 10000); i = (i + 1)) {
      const int x_148 = looplimiter0;
      const float x_150 = asfloat(x_50[0].y);
      if ((x_148 >= int(x_150))) {
        const float x_156 = asfloat(x_50[0].y);
        param_24 = (1 + int(x_156));
        const int x_159 = identity_i1_(param_24);
        pp = x_159;
        break;
      }
      looplimiter0 = (looplimiter0 + 1);
    }
  }
  if ((pp != 2)) {
    return;
  }
  count = 0;
  i_1 = 0;
  {
    [loop] for(; (i_1 < 20); i_1 = (i_1 + 1)) {
      param_25 = i_1;
      const int x_176 = search_i1_(param_25);
      result = x_176;
      switch(i_1) {
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
          if ((result == i_1)) {
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
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main() {
  const main_out inner_result = main_inner();
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000019568397DE0(32,10-21): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
internal error: compilation aborted unexpectedly

