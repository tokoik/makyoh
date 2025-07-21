#version 410 core

//
// simple.vert
//
//   単純な陰影付けを行ってオブジェクトを描画するシェーダ
//

// 全体光源
layout (std140) uniform Light
{
  vec4 lamb;                                          // 環境光成分
  vec4 ldiff;                                         // 拡散反射光成分
  vec4 lspec;                                         // 鏡面反射光成分
  vec4 lpos;                                          // 位置
};

// 変換行列
uniform mat4 mp;                                      // 投影変換行列
uniform mat4 mv;                                      // モデルビュー変換行列
uniform mat4 mn;                                      // 法線変換行列

// 頂点属性
layout (location = 0) in vec4 pv;                     // ローカル座標系の頂点位置
layout (location = 1) in vec4 nv;                     // 頂点の法線ベクトル

// ラスタライザに送る頂点属性
out vec4 vp;                                          // 視点座標系における頂点位置
out vec4 vl;                                          // 視点座標系における光源位置
out vec3 vn;                                          // 視点座標系における法線ベクトル

void main(void)
{
  // 座標計算
  vp = mv * pv;                                       // 視点座標系における頂点位置
  vl = mv * lpos;                                     // 視点座標系における光源位置
  vn = normalize((mn * nv).xyz);                      // 視点座標系における法線ベクトル

  // クリッピング座標系の頂点位置
  gl_Position = mp * vp;
}
