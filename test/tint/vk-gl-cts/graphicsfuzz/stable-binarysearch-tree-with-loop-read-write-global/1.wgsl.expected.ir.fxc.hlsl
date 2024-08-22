SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static QuicksortObject obj = (QuicksortObject)0;
static BST tree[10] = (BST[10])0;
cbuffer cbuffer_x_50 : register(b0) {
  uint4 x_50[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST node, inout int data) {
  int x_208 = data;
  node.data = x_208;
  node.leftIndex = -1;
  node.rightIndex = -1;
}

void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = (BST)0;
  int param_1 = 0;
  BST param_2 = (BST)0;
  int param_3 = 0;
  baseIndex = 0;
  {
    while(true) {
      int x_217 = baseIndex;
      int x_218 = treeIndex;
      if ((x_217 <= x_218)) {
      } else {
        break;
      }
      int x_221 = data_1;
      int x_222 = baseIndex;
      int x_224 = tree[x_222].data;
      if ((x_221 <= x_224)) {
        int x_229 = baseIndex;
        int x_231 = tree[x_229].leftIndex;
        if ((x_231 == -1)) {
          int x_236 = baseIndex;
          int x_237 = treeIndex;
          tree[x_236].leftIndex = x_237;
          int x_239 = treeIndex;
          BST v = tree[x_239];
          BST x_241 = v;
          param = x_241;
          int x_242 = data_1;
          param_1 = x_242;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
          BST v_1 = param;
          BST x_244 = v_1;
          tree[x_239] = x_244;
          return;
        } else {
          int x_246 = baseIndex;
          int x_248 = tree[x_246].leftIndex;
          baseIndex = x_248;
          {
          }
          continue;
        }
      } else {
        int x_249 = baseIndex;
        int x_251 = tree[x_249].rightIndex;
        if ((x_251 == -1)) {
          int x_256 = baseIndex;
          int x_257 = treeIndex;
          tree[x_256].rightIndex = x_257;
          int x_259 = treeIndex;
          BST v_2 = tree[x_259];
          BST x_261 = v_2;
          param_2 = x_261;
          int x_262 = data_1;
          param_3 = x_262;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
          BST v_3 = param_2;
          BST x_264 = v_3;
          tree[x_259] = x_264;
          return;
        } else {
          int x_266 = baseIndex;
          int x_268 = tree[x_266].rightIndex;
          baseIndex = x_268;
          {
          }
          continue;
        }
      }
      /* unreachable */
    }
  }
}

int identity_i1_(inout int a) {
  int x_202 = a;
  int x_203 = a;
  obj.numbers[x_202] = x_203;
  int x_206 = obj.numbers[2];
  return x_206;
}

int search_i1_(inout int t) {
  int index = 0;
  BST currentNode = (BST)0;
  int x_270 = 0;
  index = 0;
  {
    while(true) {
      int x_275 = index;
      if ((x_275 != -1)) {
      } else {
        break;
      }
      int x_278 = index;
      BST v_4 = tree[x_278];
      BST x_280 = v_4;
      currentNode = x_280;
      int x_282 = currentNode.data;
      int x_283 = t;
      if ((x_282 == x_283)) {
        int x_287 = t;
        return x_287;
      }
      int x_288 = t;
      int x_290 = currentNode.data;
      if ((x_288 > x_290)) {
        int x_296 = currentNode.rightIndex;
        x_270 = x_296;
      } else {
        int x_298 = currentNode.leftIndex;
        x_270 = x_298;
      }
      int x_299 = x_270;
      index = x_299;
      {
      }
      continue;
    }
  }
  return -1;
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (-2147483648))) : (2147483647));
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
  BST v_5 = tree[0];
  BST x_101 = v_5;
  param_4 = x_101;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  BST v_6 = param_4;
  BST x_103 = v_6;
  tree[0] = x_103;
  int x_105 = treeIndex_1;
  treeIndex_1 = (x_105 + 1);
  int x_107 = treeIndex_1;
  param_6 = x_107;
  param_7 = 5;
  insert_i1_i1_(param_6, param_7);
  int x_109 = treeIndex_1;
  treeIndex_1 = (x_109 + 1);
  int x_111 = treeIndex_1;
  param_8 = x_111;
  param_9 = 12;
  insert_i1_i1_(param_8, param_9);
  int x_113 = treeIndex_1;
  treeIndex_1 = (x_113 + 1);
  int x_115 = treeIndex_1;
  param_10 = x_115;
  param_11 = 15;
  insert_i1_i1_(param_10, param_11);
  int x_117 = treeIndex_1;
  treeIndex_1 = (x_117 + 1);
  int x_119 = treeIndex_1;
  param_12 = x_119;
  param_13 = 7;
  insert_i1_i1_(param_12, param_13);
  int x_121 = treeIndex_1;
  treeIndex_1 = (x_121 + 1);
  int x_123 = treeIndex_1;
  param_14 = x_123;
  param_15 = 8;
  insert_i1_i1_(param_14, param_15);
  int x_125 = treeIndex_1;
  treeIndex_1 = (x_125 + 1);
  int x_127 = treeIndex_1;
  param_16 = x_127;
  param_17 = 2;
  insert_i1_i1_(param_16, param_17);
  int x_129 = treeIndex_1;
  treeIndex_1 = (x_129 + 1);
  int x_131 = treeIndex_1;
  param_18 = x_131;
  param_19 = 6;
  insert_i1_i1_(param_18, param_19);
  int x_133 = treeIndex_1;
  treeIndex_1 = (x_133 + 1);
  int x_135 = treeIndex_1;
  param_20 = x_135;
  param_21 = 17;
  insert_i1_i1_(param_20, param_21);
  int x_137 = treeIndex_1;
  treeIndex_1 = (x_137 + 1);
  int x_139 = treeIndex_1;
  param_22 = x_139;
  param_23 = 13;
  insert_i1_i1_(param_22, param_23);
  pp = 0;
  looplimiter0 = 0;
  i = 0;
  {
    while(true) {
      int x_145 = i;
      if ((x_145 < 10000)) {
      } else {
        break;
      }
      int x_148 = looplimiter0;
      float x_150 = asfloat(x_50[0u].y);
      if ((x_148 >= tint_f32_to_i32(x_150))) {
        float x_156 = asfloat(x_50[0u].y);
        param_24 = (1 + tint_f32_to_i32(x_156));
        int x_159 = identity_i1_(param_24);
        pp = x_159;
        break;
      }
      int x_160 = looplimiter0;
      looplimiter0 = (x_160 + 1);
      {
        int x_162 = i;
        i = (x_162 + 1);
      }
      continue;
    }
  }
  int x_164 = pp;
  if ((x_164 != 2)) {
    return;
  }
  count = 0;
  i_1 = 0;
  {
    while(true) {
      int x_172 = i_1;
      if ((x_172 < 20)) {
      } else {
        break;
      }
      int x_175 = i_1;
      param_25 = x_175;
      int x_176 = search_i1_(param_25);
      result = x_176;
      int x_177 = i_1;
      switch(x_177) {
        case 2:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 12:
        case 13:
        case 15:
        case 17:
        {
          int x_187 = result;
          int x_188 = i_1;
          if ((x_187 == x_188)) {
            int x_192 = count;
            count = (x_192 + 1);
          }
          break;
        }
        default:
        {
          int x_181 = result;
          if ((x_181 == -1)) {
            int x_185 = count;
            count = (x_185 + 1);
          }
          break;
        }
      }
      {
        int x_194 = i_1;
        i_1 = (x_194 + 1);
      }
      continue;
    }
  }
  int x_196 = count;
  if ((x_196 == 20)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 1.0f, 1.0f);
  }
}

main_out main_inner() {
  main_1();
  main_out v_7 = {x_GLF_color};
  return v_7;
}

main_outputs main() {
  main_out v_8 = main_inner();
  main_outputs v_9 = {v_8.x_GLF_color_1};
  return v_9;
}

FXC validation failure:
C:\src\dawn\Shader@0x000002752CEE4040(41,5-15): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
C:\src\dawn\Shader@0x000002752CEE4040(41,5-15): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
C:\src\dawn\Shader@0x000002752CEE4040(112,3-20): error X3500: array reference cannot be used as an l-value; not natively addressable
C:\src\dawn\Shader@0x000002752CEE4040(259,5-15): error X3511: forced to unroll loop, but unrolling failed.

