SKIP: FAILED

#version 310 es

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


BST tree_1[10] = BST[10](BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0));
uniform buf0 x_16;
vec4 x_GLF_color = vec4(0.0f);
void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  int x_165 = data;
  tree.data = x_165;
  tree.leftIndex = -1;
  tree.rightIndex = -1;
}
void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = BST(0, 0, 0);
  int param_1 = 0;
  int x_170 = 0;
  BST param_2 = BST(0, 0, 0);
  int param_3 = 0;
  baseIndex = 0;
  {
    while(true) {
      int x_175 = baseIndex;
      int x_176 = treeIndex;
      if ((x_175 <= x_176)) {
      } else {
        break;
      }
      int x_179 = data_1;
      int x_180 = baseIndex;
      int x_182 = tree_1[x_180].data;
      if ((x_179 <= x_182)) {
        int x_187 = baseIndex;
        int x_189 = tree_1[x_187].leftIndex;
        if ((x_189 == -1)) {
          int x_194 = baseIndex;
          int x_195 = treeIndex;
          tree_1[x_194].leftIndex = x_195;
          float x_198 = x_16.injectionSwitch.x;
          float x_200 = x_16.injectionSwitch.y;
          if ((x_198 < x_200)) {
            int x_204 = treeIndex;
            BST x_206 = tree_1[x_204];
            param = x_206;
            int x_207 = data_1;
            param_1 = x_207;
            makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
            BST x_209 = param;
            tree_1[x_204] = x_209;
          }
          float x_212 = x_16.injectionSwitch.x;
          float x_214 = x_16.injectionSwitch.y;
          if ((x_212 < x_214)) {
            return;
          }
        } else {
          int x_218 = baseIndex;
          int x_220 = tree_1[x_218].leftIndex;
          baseIndex = x_220;
          {
          }
          continue;
        }
      } else {
        float x_222 = x_16.injectionSwitch.x;
        float x_224 = x_16.injectionSwitch.y;
        if ((x_222 < x_224)) {
          int x_229 = baseIndex;
          int x_231 = tree_1[x_229].rightIndex;
          x_170 = x_231;
        } else {
          int x_232 = baseIndex;
          int x_234 = tree_1[x_232].rightIndex;
          x_170 = x_234;
        }
        int x_235 = x_170;
        if ((x_235 == -1)) {
          int x_240 = baseIndex;
          int x_241 = treeIndex;
          tree_1[x_240].rightIndex = x_241;
          int x_243 = treeIndex;
          BST x_245 = tree_1[x_243];
          param_2 = x_245;
          int x_246 = data_1;
          param_3 = x_246;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
          BST x_248 = param_2;
          tree_1[x_243] = x_248;
          return;
        } else {
          int x_250 = baseIndex;
          int x_252 = tree_1[x_250].rightIndex;
          baseIndex = x_252;
          {
          }
          continue;
        }
      }
      float x_254 = x_16.injectionSwitch.x;
      float x_256 = x_16.injectionSwitch.y;
      if ((x_254 > x_256)) {
        return;
      }
      {
      }
      continue;
    }
  }
}
int search_i1_(inout int t) {
  int index = 0;
  BST currentNode = BST(0, 0, 0);
  int x_261 = 0;
  index = 0;
  {
    while(true) {
      int x_266 = index;
      if ((x_266 != -1)) {
      } else {
        break;
      }
      int x_269 = index;
      BST x_271 = tree_1[x_269];
      currentNode = x_271;
      int x_273 = currentNode.data;
      int x_274 = t;
      if ((x_273 == x_274)) {
        int x_278 = t;
        return x_278;
      }
      int x_279 = t;
      int x_281 = currentNode.data;
      if ((x_279 > x_281)) {
        int x_287 = currentNode.rightIndex;
        x_261 = x_287;
      } else {
        int x_289 = currentNode.leftIndex;
        x_261 = x_289;
      }
      int x_290 = x_261;
      index = x_290;
      {
      }
      continue;
    }
  }
  return -1;
}
void main_1() {
  int treeIndex_1 = 0;
  BST param_4 = BST(0, 0, 0);
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
  BST x_91 = tree_1[0];
  param_4 = x_91;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  BST x_93 = param_4;
  tree_1[0] = x_93;
  int x_95 = treeIndex_1;
  treeIndex_1 = (x_95 + 1);
  int x_97 = treeIndex_1;
  param_6 = x_97;
  param_7 = 5;
  insert_i1_i1_(param_6, param_7);
  int x_99 = treeIndex_1;
  treeIndex_1 = (x_99 + 1);
  int x_101 = treeIndex_1;
  param_8 = x_101;
  param_9 = 12;
  insert_i1_i1_(param_8, param_9);
  int x_103 = treeIndex_1;
  treeIndex_1 = (x_103 + 1);
  int x_105 = treeIndex_1;
  param_10 = x_105;
  param_11 = 15;
  insert_i1_i1_(param_10, param_11);
  int x_107 = treeIndex_1;
  treeIndex_1 = (x_107 + 1);
  int x_109 = treeIndex_1;
  param_12 = x_109;
  param_13 = 7;
  insert_i1_i1_(param_12, param_13);
  int x_111 = treeIndex_1;
  treeIndex_1 = (x_111 + 1);
  int x_113 = treeIndex_1;
  param_14 = x_113;
  param_15 = 8;
  insert_i1_i1_(param_14, param_15);
  int x_115 = treeIndex_1;
  treeIndex_1 = (x_115 + 1);
  int x_117 = treeIndex_1;
  param_16 = x_117;
  param_17 = 2;
  insert_i1_i1_(param_16, param_17);
  int x_119 = treeIndex_1;
  treeIndex_1 = (x_119 + 1);
  int x_121 = treeIndex_1;
  param_18 = x_121;
  param_19 = 6;
  insert_i1_i1_(param_18, param_19);
  int x_123 = treeIndex_1;
  treeIndex_1 = (x_123 + 1);
  int x_125 = treeIndex_1;
  param_20 = x_125;
  param_21 = 17;
  insert_i1_i1_(param_20, param_21);
  int x_127 = treeIndex_1;
  treeIndex_1 = (x_127 + 1);
  int x_129 = treeIndex_1;
  param_22 = x_129;
  param_23 = 13;
  insert_i1_i1_(param_22, param_23);
  count = 0;
  i = 0;
  {
    while(true) {
      int x_135 = i;
      if ((x_135 < 20)) {
      } else {
        break;
      }
      int x_138 = i;
      param_24 = x_138;
      int x_139 = search_i1_(param_24);
      result = x_139;
      int x_140 = i;
      switch(x_140) {
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
          int x_150 = result;
          int x_151 = i;
          if ((x_150 == x_151)) {
            int x_155 = count;
            count = (x_155 + 1);
          }
          break;
        }
        default:
        {
          int x_144 = result;
          if ((x_144 == -1)) {
            int x_148 = count;
            count = (x_148 + 1);
          }
          break;
        }
      }
      {
        int x_157 = i;
        i = (x_157 + 1);
      }
      continue;
    }
  }
  int x_159 = count;
  if ((x_159 == 20)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
