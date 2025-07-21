#pragma once

///
/// メニューの描画クラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date August 15, 2025
///

// 宿題用補助プログラムのラッパー
#include "GgApp.h"

// 構成データ
#include "Config.h"

// ファイルダイアログ
#include "nfd.h"

///
/// メニューの描画
///
class Menu
{
  // 図形の描画クラスから参照する
  friend class Scene;

  // オリジナルの構成データ
  const Config& defaults;

  // 構成データのコピー
  Config settings;

  // 設定ファイルを読み込む
  void loadConfig();

  // 設定ファイルを書き出す
  void saveConfig() const;

  // 全体光源データ
  std::unique_ptr<const GgSimpleShader::LightBuffer> light;

  // 全体光源の強度と位置を設定する
  void setLight();

  // 投影光源データ
  std::unique_ptr<const GgSimpleShader::LightBuffer> illuminant;

  // 投影光源データを設定する
  void setIlluminantIntensity();

  // 投影光源マップのテクスチャ
  GLuint illuminantMap;

  // 投影光源マップを読み込む
  void loadIlluminantMap();

  // 投影光源の姿勢
  GgMatrix illuminantPose;

  // 投影光源の姿勢を設定する
  void setIlluminantPose();

  // 鏡の材質データのユニフォームバッファオブジェクト
  const GLuint mirrorMaterialBuffer;

  // 鏡の材質を設定する
  void setMirrorMaterial();

  // 鏡の高さマップのテクスチャ
  GLuint mirrorHeightMap;

  // 鏡の高さマップを読み込む
  void loadMirrorHeightMap();

  // 鏡の標本点のユニフォームバッファオブジェクト
  GLuint mirrorSampleBuffer;

  // 鏡の標本点を生成する
  void generateMirrorSample(int samples);

  // 鏡の姿勢
  GgMatrix mirrorPose;

  // 鏡の姿勢を設定する
  void setMirrorPose();

  // 受光面の形状ファイル名
  std::unique_ptr<const GgSimpleObj> receiverModel;

  // 受光面からの視界
  GgMatrix receiverView;

  // 受光面の姿勢
  GgMatrix receiverPose;

  // 受光面の姿勢を設定する
  void setReceiverPose();

  // 受光面の形状ファイルを読み込む
  void loadReceiverModel();

  // ファイルパスを取得する
  bool getFilePath(std::string& path, const nfdfilteritem_t* filter);

public:

  // 描画モード
  enum DrawMode
  {
    DRAW_MIRROR = 0,
    DRAW_RECEIVER
  };

private:

  // 描画モード
  DrawMode drawMode;

public:

  ///
  /// コンストラクタ
  ///
  /// @param config 構成データ
  ///
  Menu(const Config& config);

  ///
  /// コピーコンストラクタは使用しない
  ///
  /// @param menu コピー元のメニュー
  ///
  Menu(const Menu& menu) = delete;

  ///
  /// ムーブコンストラクタはデフォルトのものを使用する
  ///
  /// @param menu ムーブ元のメニュー
  ///
  Menu(Menu&& menu) = default;

  ///
  /// デストラクタ.
  ///
  virtual ~Menu();

  ///
  /// 代入演算子は使用しない
  ///
  /// @param menu 代入元のメニュー
  ///
  Menu& operator=(const Menu& menu) = delete;

  ///
  /// ムーブ代入演算子はデフォルトのものを使用する
  ///
  /// @param menu ムーブ代入元のメニュー
  ///
  Menu& operator=(Menu&& menu) = default;

  ///
  /// 全体光源データを取り出す
  ///
  /// @return 全体光源データ
  ///
  const auto& getLight() const
  {
    return *light;
  }

  ///
  /// 投影光源データを取り出す
  ///
  /// @return 投影光源データ
  ///
  const auto& getIlluminantIntensity() const
  {
    return *illuminant;
  }

  ///
  /// 投影光源マップのテクスチャを取り出す
  ///
  /// @return 投影光源マップのテクスチャ
  ///
  auto getIlluminantMap() const
  {
    return illuminantMap;
  }

  ///
  /// 投影光源の姿勢を取り出す
  ///
  const auto& getIlluminantPose() const
  {
    return illuminantPose;
  }

  ///
  /// 鏡の高さマップのテクスチャを取り出す
  ///
  auto getHeightMap() const
  {
    return mirrorHeightMap;
  }

  ///
  /// 鏡の姿勢を取り出す
  ///
  const auto& getMirrorPose() const
  {
    return mirrorPose;
  }

  ///
  /// 鏡の材質のユニフォームバッファオブジェクトを結合ポイントに結合する
  ///
  void bindMirrorMaterial(GLuint bindingPoint) const
  {
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, mirrorMaterialBuffer);
  }

  ///
  /// 鏡の標本点のユニフォームバッファオブジェクトを結合ポイントに結合する
  ///
  void bindMirrorSample(GLuint bindingPoint) const
  {
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, mirrorSampleBuffer);
  }

  ///
  /// 鏡の標本数を取り出す
  ///
  auto getMirrorSampleCount() const
  {
    return settings.mirrorSampleCount;
  }  

  ///
  /// 鏡の高さマップのスケールを取り出す
  ///
  auto getMirrorHeightScale() const
  {
    return settings.mirrorHeightScale;
  }

  ///
  /// 受光面の視界を取り出す
  ///
  const auto& getReceiverView() const
  {
    return receiverView;
  }

  ///
  /// 受光面の姿勢を取り出す
  ///
  const auto& getReceiverPose() const
  {
    return receiverPose;
  }

  ///
  /// モデルデータを取り出す
  ///
  const auto& getReceiverModel() const
  {
    return *receiverModel;
  }

  // 描画モードを取り出す
  auto getDrawMode() const
  {
    return drawMode;
  }

  ///
  /// 描画する
  ///
  void draw();
};
