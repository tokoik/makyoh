#pragma once

///
/// 矩形クラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date August 12, 2025
///

// 補助プログラム
#include "gg.h"
using namespace gg;

///
/// 矩形クラス
///
class Rect
{
  // 頂点配列オブジェクト
  GLuint vao;

public:

  ///
  /// コンストラクタ
  ///
  Rect();

  ///
  /// コピーコンストラクタは使用しない
  ///
  Rect(const Rect& rectangle) = delete;

  ///
  /// ムーブコンストラクタはデフォルトのものを使用する
  ///
  Rect(Rect&& rectangle) = default;

  ///
  /// デストラクタ
  ///
  virtual ~Rect();

  ///
  /// 代入演算子は使用しない
  ///
  Rect& operator=(const Rect& rectangle) = delete;

  ///
  /// ムーブ代入演算子はデフォルトのものを使用する
  ///
  Rect& operator=(Rect&& rectangle) = default;

  ///
  /// 描画する
  ///
  void draw() const;
};
