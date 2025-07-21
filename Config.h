#pragma once

///
/// 構成データクラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///

// 補助プログラム
#include "gg.h"
using namespace gg;

// 構成ファイルの読み取り補助
#include "parseconfig.h"

// 投影光源マップを使わない場合は定義する
#undef USE_ILLUMINANT_COLOR

// 鏡の標本点数の上限
constexpr auto MAX_MIRROR_SAMPLES{ 1000 };

///
/// 構成データ
///
class Config
{
  // メニュークラスから参照する
  friend class Menu;

  // ウィンドウサイズ
  std::array<GLsizei, 2> windowSize;

  // メニューフォント名
  std::string menuFont;

  // メニューフォントサイズ
  float menuFontSize;

  // 全体光源の色
  GgVector lightColor;

  // 全体光源の強度
  GLfloat lightIntensity;

  // 全体光源の環境光成分
  GLfloat lightAmbient;

  // 全体光源の位置
  GgVector lightPosition;

  // 投影光源の色
  GgVector illuminantColor;

  // 投影光源データ
  GLfloat illuminantIntensity;

  // 投影光源の環境光成分
  GLfloat illuminantAmbient;

  // 投影光源の位置
  GgVector illuminantPosition;

  // 投影光源の目標
  GgVector illuminantTarget;

  // 投影光源の広がり
  GLfloat illuminantSpread;

  // 投影光源マップのファイル名
  std::string illuminantMap;

  // 鏡の拡散反射係数
  GgVector mirrorMaterialDiffuse;

  // 鏡の鏡面反射係数
  GgVector mirrorMaterialSpecular;

  // 鏡の輝き係数
  GLfloat mirrorMaterialShininess;

  // 鏡の位置
  GgVector mirrorPosition;

  // 鏡の目標
  GgVector mirrorTarget;

  // 鏡の高さマップのファイル名
  std::string mirrorHeightMap;

  // 鏡の高さマップのスケール
  GLfloat mirrorHeightScale;

  // 鏡のサンプル点数
  int mirrorSampleCount;

  // 受光面の形状ファイル名
  std::string receiverModel;

  // 受光面の位置
  GgVector receiverPosition;

  // 受光面の回転とスケール
  GgVector receiverOrientation;

public:

  ///
  /// コンストラクタ
  ///
  Config();

  ///
  /// ファイルから構成データを読み込むコンストラクタ
  ///
  /// @param filename 読み込む構成ファイル名
  ///
  Config(const std::string& filename);

  ///
  /// デストラクタ
  ///
  virtual ~Config();

  ///
  /// ウィンドウの横幅を得る
  ///
  /// @return ウィンドウの横幅
  ///
  auto getWidth() const
  {
    return windowSize[0];
  }

  ///
  /// ウィンドウの高さを得る
  ///
  /// @return ウィンドウの高さ
  ///
  auto getHeight() const
  {
    return windowSize[1];
  }

  ///
  /// 構成ファイルを読み込む
  ///
  /// @param filename 読み込む構成ファイル名
  ///
  bool load(const std::string& filename);

  ///
  /// 構成ファイルを書き出す
  ///
  /// @param filename 書き出す構成ファイル名
  ///
  bool save(const std::string& filename) const;
};
