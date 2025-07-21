#version 410 core

//
// rectangle.frag
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

// 材質
layout (std140) uniform Material
{
  vec4 kamb;                                          // 環境光の反射係数
  vec4 kdiff;                                         // 拡散反射係数
  vec4 kspec;                                         // 鏡面反射係数
  float kshi;                                         // 輝き係数
};

// 鏡
layout (std140) uniform Mirror
{
  vec4 mamb;                                          // 環境光の反射係数
  vec4 mdiff;                                         // 拡散反射係数
  vec4 mspec;                                         // 鏡面反射係数
  float mshi;                                         // 輝き係数
};

// パラメータ
uniform float scale;                                  // 鏡の高さマップのスケール

// テクスチャ
uniform sampler2D height;                             // 鏡の高さマップ
uniform sampler2D color;                              // 投影光源マップ

// 変換行列
uniform mat4 mn;                                      // 法線変換行列
uniform mat4 ml;                                      // 光源の姿勢行列

// ラスタライザから受け取る頂点属性の補間値
in vec2 tc;                                           // テクスチャ座標
in vec4 vp;                                           // 視点座標系における頂点位置
in vec4 vl;                                           // 視点座標系における光源位置

// フレームバッファに出力するデータ
layout (location = 0) out vec4 fc;                    // フラグメントの色

void main(void)
{
  // テクスチャ座標を[-1, 1]に変換
  vec2 radius = tc * vec2(2.0, -2.0) - vec2(1.0, -1.0);

  // 円形の鏡の範囲外は捨てる
  if (dot(radius, radius) > 1.0) discard;

  // 視点座標系における法線ベクトル
  vec3 dx = vec3(1.0, 0.0, (textureOffset(height, tc, ivec2(1, 0)).r - textureOffset(height, tc, ivec2(-1, 0)).r) * scale);
  vec3 dy = vec3(0.0, 1.0, (textureOffset(height, tc, ivec2(0, 1)).r - textureOffset(height, tc, ivec2(0, -1)).r) * scale);
  vec3 n = mat3(mn) * normalize(cross(dx, dy));

  // 視点座標系における光線ベクトル
  vec3 l = normalize((vl * vp.w - vp * vl.w).xyz);

  // 視点座標系における視線ベクトル
  vec3 v = normalize(vp.xyz);

  // 視点座標系における中間ベクトル
  vec3 h = normalize(l - v);

  // 全体光源の陰影計算
  vec4 iamb = mamb * lamb;
  vec4 idiff = max(dot(n, l), 0.0) * mdiff * ldiff * 0.318309886;
  vec4 ispec = (mshi + 8.0) * pow(max(dot(n, h), 0.0), mshi) * mspec * lspec * 0.0397887358;

  // 全体光源の反射光強度
  fc = iamb + idiff + ispec;

  // 視線の反射ベクトル
  vec3 d = reflect(vp.xyz, n);

  // 法線ベクトルと視線の反射ベクトルの内積
  float k = dot(ml[2].xyz, d);

  // 法線ベクトルと視線の反射ベクトルの内積が向かい合っていなければ映り込みは無い
  if (k >= 0.0) return;

  // 光源の中心から反射位置に向かうベクトル（元の式とは向きを反転している）
  vec3 t = (ml[3] * vp.w - vp * ml[3].w).xyz;

  // 反射位置から光源の中心に向かうベクトルと視線の反射ベクトルの外積
  vec3 m = cross(t, d);

  // 交点のパラメータ座標（テクスチャ座標なので Y 軸は反転）
  vec3 p = vec3(dot(t, ml[2].xyz), dot(m, ml[1].xyz), dot(m, ml[0].xyz)) / k;

  // 交点までの距離が負なら反対側なので環境光のみにする
  if (p.x < 0.0) return;

  // 矩形と交差していなければ環境光のみにする
  if (any(lessThan(vec4(1.0 + p.yz, 1.0 - p.yz), vec4(0.0)))) return;

  // 光源色
  vec4 lc = texture(color, p.yz * 0.5 + 0.5);

  // 画素の陰影を求める
  fc += mspec * lc;
}
