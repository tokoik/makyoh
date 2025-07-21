#version 410 core

//
// simple.frag
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

// 標本点
layout (std140) uniform Sample
{
  vec4 point[1000];                                   // 標本点の位置
};

// パラメータ
uniform int samples;                                  // 標本点の数
uniform float scale;                                  // 鏡の高さスケール

// テクスチャ
uniform sampler2D height;                             // 鏡の高さマップ
uniform sampler2D color;                              // 投影光源マップ

// 変換行列
uniform mat4 mn;                                      // 法線変換行列
uniform mat4 mm;                                      // 鏡の姿勢行列
uniform mat4 ml;                                      // 投影光源の姿勢行列

// ラスタライザから受け取る頂点属性の補間値
in vec4 vp;                                           // 視点座標系における頂点位置
in vec4 vl;                                           // 視点座標系における全体光源位置
in vec3 vn;                                           // 視点座標系における法線ベクトル

// フレームバッファに出力するデータ
layout (location = 0) out vec4 fc;                    // フラグメントの色

// 受光面上の点 vp から direction 方向を見た放射輝度
vec4 radiance(in vec3 direction, in vec3 view, in vec3 normal)
{
  // 視点座標系の受光面の位置から鏡の中心に向かうベクトル
  vec3 t0 = (vp * mm[3].w - mm[3] * vp.w).xyz;

  // 視点座標系の受光面の位置から鏡の中心に向かうベクトルと鏡面上の１点に向かうベクトルの外積
  vec3 m0 = cross(t0, direction);

  // 鏡の法線ベクトルと鏡面上の１点に向かうベクトルの内積
  float k0 = dot(mm[2].xyz, direction);

  // 鏡の交点のパラメータ座標（テクスチャ座標なので Y 軸は反転）
  vec3 p0 = vec3(-dot(t0, mm[2].xyz), -dot(m0, mm[1].xyz), dot(m0, mm[0].xyz)) / k0;

  // 鏡の交点までの距離が負なら反対側なので反射光はない
  if (p0.x < 0.0) return vec4(0.0);

  // 鏡と交差していなければ反射光はない
  if (dot(p0.yz, p0.yz) > 1.0) return vec4(0.0);

  // 鏡の交点の視点座標系の位置
  vec4 v0 = mm * vec4(p0.yz, 0.0, 1.0);

  // 視点座標系における鏡の法線ベクトル
  float dx = textureOffset(height, p0.yz, ivec2(-1, 0)).r - textureOffset(height, p0.yz, ivec2(1, 0)).r;
  float dy = textureOffset(height, p0.yz, ivec2(0, -1)).r - textureOffset(height, p0.yz, ivec2(0, 1)).r;
  vec3 n = mat3(mn) * (normalize(vec3(vec2(dx, dy) * scale, 1.0)));

  // 鏡の交点の視点座標系における視線ベクトル
  vec3 v = normalize((v0 * vp.w - vp * v0.w).xyz);

  // 鏡の交点の視点座標系における視線ベクトルの反射ベクトル
  vec3 d = reflect(v.xyz, n);

  // 鏡の交点の視点座標系における法線ベクトルと視線の反射ベクトルの内積
  float k = dot(ml[2].xyz, d);

  // 鏡の反射光強度
  vec4 intensity = mamb * lamb;

  // 鏡の交点の視点座標系における法線ベクトルと視線の反射ベクトルの内積が向かい合っていなければ映り込みは無い
  if (k >= 0.0) return intensity;

  // 鏡の交点の視点座標系における光線ベクトル
  vec3 l = normalize((vl * v0.w - v0 * vl.w).xyz);

  // 鏡の視点座標系における中間ベクトル
  vec3 h = normalize(l - v);

  // 全体光源の陰影計算
  vec4 idiff = max(dot(n, l), 0.0) * mdiff * ldiff * 0.318309886;
  vec4 ispec = (mshi + 8.0) * pow(max(dot(n, h), 0.0), mshi) * mspec * lspec * 0.0397887358;

  // 鏡の全体光源による反射光強度
  intensity += idiff + ispec;

  // 投影光源の中心から鏡の交点に向かうベクトル（元の式とは向きを反転している）
  vec3 t = (ml[3] * v0.w - v0 * ml[3].w).xyz;

  // 反射位置から投影光源の中心に向かうベクトルと視線の反射ベクトルの外積
  vec3 m = cross(t, d);

  // 投影光源の交点のパラメータ座標（テクスチャ座標なので Y 軸は反転）
  vec3 p = vec3(dot(t, ml[2].xyz), dot(m, ml[1].xyz), dot(m, ml[0].xyz)) / k;

  // 投影光源の交点までの距離が負なら反対側なので全体光源の反射光のみにする
  if (p.x < 0.0) return intensity;

  // 投影光源と交差していなければ全体光源の反射光のみにする
  if (any(lessThan(vec4(1.0 + p.yz, 1.0 - p.yz), vec4(0.0)))) return intensity;

  // 投影光源色
  vec4 lc = texture(color, p.yz * 0.5 + 0.5);

  // 視点座標系の受光面の位置から鏡面上の１点に向かうベクトルと視線ベクトルの中間ベクトル
  vec3 halfway = normalize(direction - view);

  // 受光面の鏡の反射光による陰影計算
  vec4 mdiff = max(dot(normal, direction), 0.0) * kdiff * 0.318309886;
  vec4 mspec = (kshi + 8.0) * pow(max(dot(normal, halfway), 0.0), kshi) * kspec * 0.0397887358;

  // 画素の陰影を求める
  return intensity + (kamb + mdiff + mspec) * lc;
}

void main(void)
{
  // 視点座標系における各種ベクトル
  vec3 n = normalize(vn);                             // 視点座標系における法線ベクトル
  vec3 l = normalize((vl * vp.w - vp * vl.w).xyz);    // 視点座標系における光線ベクトル
  vec3 v = normalize(vp.xyz);                         // 視点座標系における視線ベクトル
  vec3 h = normalize(l - v);                          // 視点座標系における中間ベクトル

  // 陰影計算
  vec4 iamb = kamb * lamb;
  vec4 idiff = max(dot(n, l), 0.0) * kdiff * ldiff;
  vec4 ispec = pow(max(dot(n, h), 0.0), kshi) * kspec * lspec;

  // 投影光源による反射光強度
  vec4 intensity = vec4(0.0);

  // 各標本点における反射光強度を合計する
  for (int i = 0; i < samples; ++i)
  {
    // 視点座標系における標本点の位置
    vec4 sp = mm * vec4(point[i]);

    // 観測位置から視点座標系における標本点に向かうベクトル
    vec3 direction = (sp * vp.w - vp * sp.w).xyz;

    // 標本点からの放射輝度を加算
    intensity += radiance(direction, v, n);
  }

  // 標本点の数で割る
  intensity /= float(samples);

  // 画素の陰影を求める
  fc = intensity + iamb + idiff + ispec;
}
