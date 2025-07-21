#version 410 core

//
// rectangle.vert
//
//   矩形を描画するシェーダ
//

// 光源
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

// ラスタライザに送る頂点属性
out vec2 tc;                                          // テクスチャ座標
out vec4 vp;                                          // 視点座標系における頂点位置
out vec4 vl;                                          // 視点座標系における光源位置

void main(void)
{
  // テクスチャ座標を求める
  tc = vec2(gl_VertexID % 2, gl_VertexID / 2);
  
  // テクスチャ座標からローカル座標系の頂点位置を求める
  vec4 pv = vec4(tc * 2.0 - 1.0, 0.0, 1.0);

  // 座標計算
  vp = mv * pv;                                       // 視点座標系における頂点位置
  vl = mv * lpos;                                     // 視点座標系における光源位置

  // クリッピング座標系の頂点位置
  gl_Position = mp * vp;
}
