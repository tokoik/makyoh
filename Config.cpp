///
/// 設定構造体の実装
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///
#include "Config.h"

// 標準ライブラリ
#include <fstream>

//
// コンストラクタ
//
Config::Config() :
  windowSize{ 1280, 960 },
  menuFont{ "Mplus1-Regular.ttf" },
  menuFontSize{ 20.0f },
  lightColor{ 1.0f, 1.0f, 1.0f, 0.0f },
  lightIntensity{ 1.0f },
  lightAmbient{ 0.1f },
  lightPosition{ 2.0f, 8.0f, 1.0f, 1.0f },
  illuminantColor{ 1.0f, 1.0f, 1.0f, 0.0f },
  illuminantIntensity{ 1.0f },
  illuminantAmbient{ 0.1f },
  illuminantPosition{ 0.0f, 0.0f, 2.0f, 1.0f },
  illuminantTarget{ 0.0f, 0.0f, 0.0f, 1.0f },
  illuminantSpread{ 100.0f },
  illuminantMap{ "illuminant_map.png" },
  mirrorMaterialDiffuse{ 0.1f, 0.1f, 0.1f, 0.0f },
  mirrorMaterialSpecular{ 0.9f, 0.9f, 0.9f, 0.0f },
  mirrorMaterialShininess{ 100.0f },
  mirrorPosition{ 0.0f, 0.0f, 0.0f, 1.0f },
  mirrorTarget{ 0.0f, 0.0f, 1.0f, 1.0f },
  mirrorHeightMap{ "height_map_128.png" },
  mirrorHeightScale{ 1.0f },
  mirrorSampleCount{ 100 },
  receiverModel{ "logo.obj" },
  receiverPosition{ 0.0f, 0.0f, 5.0f, 1.0f },
  receiverOrientation{ 0.0f, 0.0f, 0.0f, 1.0f }
{
}

//
// ファイルから構成データを読み込むコンストラクタ
//
Config::Config(const std::string& filename) :
  Config{}
{
  // 構成ファイルが読み込めなかったらデフォルト値の構成ファイルを作る
  if (!load(filename)) save(filename);
}

//
// デストラクタ
//
Config::~Config()
{
}

//
// 設定ファイルを読み込む
//
bool Config::load(const std::string& filename)
{
  // 構成ファイルを開く
  std::ifstream file{ Utf8ToTChar(filename) };

  // 開けなかったらエラー
  if (!file) return false;

  // JSON の読み込み
  picojson::value value;
  file >> value;
  file.close();

  // JSON として読めていなかったらエラー
  if (!value.evaluate_as_boolean()) return false;

  // 構成データの取り出し
  const auto& object{ value.get<picojson::object>() };

  // オブジェクトが空だったらエラー
  if (object.empty()) return false;

  //
  // 構成データの読み込み
  //

  // ウィンドウサイズ
  getValue(object, "window_size", windowSize);

  // メニューフォント
  getString(object, "menu_font", menuFont);

  // メニューフォントサイズ
  getValue(object, "menu_font_size", menuFontSize);

  // 全体光源
  getVector(object, "light_color", lightColor);
  getValue(object, "light_intensity", lightIntensity);
  getValue(object, "light_ambient", lightAmbient);
  getVector(object, "light_position", lightPosition);

  // 投影光源
  getVector(object, "illuminant_color", illuminantColor);
  getValue(object, "illuminant_intensity", illuminantIntensity);
  getValue(object, "illuminant_ambient", illuminantAmbient);
  getVector(object, "illuminant_position", illuminantPosition);
  getVector(object, "illuminant_target", illuminantTarget);
  getValue(object, "illuminant_spread", illuminantSpread);

  // 投影光源マップ
  getString(object, "illuminant_map", illuminantMap);

  // 鏡
  getVector(object, "mirror_diffuse", mirrorMaterialDiffuse);
  getVector(object, "mirror_specular", mirrorMaterialSpecular);
  getValue(object, "mirror_shininess", mirrorMaterialShininess);
  getVector(object, "mirror_position", mirrorPosition);
  getVector(object, "mirror_target", mirrorTarget);
  getValue(object, "mirror_sample_count", mirrorSampleCount);
  if (mirrorSampleCount <= 0) mirrorSampleCount = 1;
  if (mirrorSampleCount > MAX_MIRROR_SAMPLES) mirrorSampleCount = MAX_MIRROR_SAMPLES;

  // 鏡の高さマップ
  getString(object, "mirror_height_map", mirrorHeightMap);

  // 鏡の高さマップのスケール
  getValue(object, "mirror_height_scale", mirrorHeightScale);

  // 受光面
  getString(object, "receiver_model", receiverModel);
  getVector(object, "receiver_position", receiverPosition);
  getVector(object, "receiver_orientation", receiverOrientation);

  // オブジェクトが空だったらエラー
  if (object.empty()) return false;

  return true;
}

//
// 設定ファイルを書き出す
//
bool Config::save(const std::string& filename) const
{
  // 構成ファイルを開く
  std::ofstream file{ Utf8ToTChar(filename) };

  // 開けなかったらエラー
  if (!file) return false;

  // 構成データの書き出しに使うオブジェクト
  picojson::object object;

  //
  // 構成データの書き出し
  //

  // ウィンドウサイズ
  setValue(object, "window_size", windowSize);

  // メニューフォント
  setString(object, "menu_font", menuFont);

  // メニューフォントサイズ
  setValue(object, "menu_font_size", menuFontSize);

  // 全体光源
  setVector(object, "light_color", lightColor);
  setValue(object, "light_intensity", lightIntensity);
  setValue(object, "light_ambient", lightAmbient);
  setVector(object, "light_position", lightPosition);

  // 投影光源
  setVector(object, "illuminant_color", illuminantColor);
  setValue(object, "illuminant_intensity", illuminantIntensity);
  setValue(object, "illuminant_ambient", illuminantAmbient);
  setVector(object, "illuminant_position", illuminantPosition);
  setVector(object, "illuminant_target", illuminantTarget);
  setValue(object, "illuminant_spread", illuminantSpread);

  // 投影光源マップ
  setString(object, "illuminant_map", illuminantMap);

  // 鏡
  setVector(object, "mirror_diffuse", mirrorMaterialDiffuse);
  setVector(object, "mirror_specular", mirrorMaterialSpecular);
  setValue(object, "mirror_shininess", mirrorMaterialShininess);
  setVector(object, "mirror_position", mirrorPosition);
  setVector(object, "mirror_target", mirrorTarget);
  setValue(object, "mirror_sample_count", mirrorSampleCount);

  // 鏡の高さマップ
  setString(object, "mirror_height_map", mirrorHeightMap);

  // 鏡の高さマップのスケール
  setValue(object, "mirror_height_scale", mirrorHeightScale);

  // 受光面
  setString(object, "receiver_model", receiverModel);
  setVector(object, "receiver_position", receiverPosition);
  setVector(object, "receiver_orientation", receiverOrientation);

  // 構成出データをシリアライズして JSON で保存
  picojson::value v{ object };
  file << v.serialize(true);
  file.close();

  return true;
}
