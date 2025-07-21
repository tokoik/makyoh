///
/// メニューの描画クラスの実装
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///
#include "Menu.h"

// 乱数
#include <random>

// 画像の読み込みライブラリ
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

// JSON ファイル名のフィルタ
constexpr nfdfilteritem_t jsonFilter[]{ { "JSON", "json" } };

// 画像ファイル名のフィルタ
constexpr nfdfilteritem_t imageFilter[]{ "Images", "png,gif,jpg,jpeg,jfif,bmp,dib,tga,psd,pgm,ppm" };

// 形状ファイル名のフィルタ
constexpr nfdfilteritem_t shapeFilter[]{ "Wavefront OBJ", "obj" };

// エラーが無ければ nullptr
const char* errorMessage{ nullptr };

///
/// 姿勢を設定する
///
/// @param matrix 姿勢行列
/// @param position 中心位置
/// @param target 目標位置
/// @param up 上方向ベクトル (デフォルトは Y 軸方向)
///
static void setPose(GgMatrix& matrix, const GgVector& position, const GgVector& target,
  const GgVector& up = GgVector{ 0.0f, 1.0f, 0.0f, 0.0f })
{
  // 中心位置から目標位置へのベクトルを z 軸とする
  GgVector z
  {
    target[0] * position[3] - position[0] * target[3],
    target[1] * position[3] - position[1] * target[3],
    target[2] * position[3] - position[2] * target[3],
    0.0f
  };

  // z 軸の長さを求める
  const auto lz{ z.length3() };

  // z 軸の長さがゼロなら単位行列を返す
  if (fabs(lz) < std::numeric_limits<float>::epsilon())
  {
    matrix.loadIdentity();
    return;
  }

  // z 軸と上方向 up に直交するベクトルを x 軸とする
  auto x{ ggCross(up, z) };

  // x 軸の長さを求める
  const auto lx{ x.length3() };

  // x 軸の長さがゼロなら単位行列を返す
  if (fabs(lx) < std::numeric_limits<float>::epsilon())
  {
    matrix.loadIdentity();
    return;
  }

  // z 軸と x 軸に直交するベクトルを y 軸とする
  auto y{ ggCross(z, x) };

  // y 軸の長さを求める
  const auto ly{ y.length3() };

  // y 軸の長さがゼロなら単位行列を返す
  if (fabs(ly) < std::numeric_limits<float>::epsilon())
  {
    matrix.loadIdentity();
    return;
  }

  // 正規化する
  x /= lx;
  y /= ly;
  z /= lz;

  // 姿勢行列を設定する
  matrix[ 0] = x[0];
  matrix[ 1] = x[1];
  matrix[ 2] = x[2];
  matrix[ 3] = 0.0f;

  matrix[ 4] = y[0];
  matrix[ 5] = y[1];
  matrix[ 6] = y[2];
  matrix[ 7] = 0.0f;

  matrix[ 8] = z[0];
  matrix[ 9] = z[1];
  matrix[10] = z[2];
  matrix[11] = 0.0f;

  matrix[12] = position[0] / position[3];
  matrix[13] = position[1] / position[3];
  matrix[14] = position[2] / position[3];
  matrix[15] = 1.0f;
}

///
/// テクスチャを作成して画像ファイルを読み込む
///
/// @param name 読み込む画像ファイル名
/// @return テクスチャ名
///
static GLuint loadImage(const std::string& name)
{
  // 画像サイズ
  int width, height;

  // 画像のチャンネル数
  int channels;

  // 画像を読み込む
  const auto image{ stbi_load(name.c_str(), &width, &height, &channels, 0) };

  // 画像が読み込めなかったら戻る
  if (!image) return 0;

  // 画像のフォーマットは読み込んだファイルに合わせる
  GLenum format[]{ GL_RGBA, GL_RED, GL_RG, GL_RGB, GL_RGBA };

  // テクスチャに読み込む
  const auto tex{ ggLoadTexture(image, width, height,
    format[channels], GL_UNSIGNED_BYTE, format[channels], GL_CLAMP_TO_EDGE, false)};

  // 読み込んだ画像のメモリを開放する
  stbi_image_free(image);

  // テクスチャ名を返す
  return tex;
}

//
// コンストラクタ
//
Menu::Menu(const Config& config) :
  defaults{ config },
  settings{ config },
  light{ std::make_unique<GgSimpleShader::LightBuffer>() },
  illuminant{ std::make_unique<GgSimpleShader::LightBuffer>() },
  illuminantMap{ loadImage(config.illuminantMap) },
  mirrorMaterialBuffer{ [] { GLuint ubo; glGenBuffers(1, &ubo); return ubo; }() },
  mirrorHeightMap{ loadImage(config.mirrorHeightMap) },
  mirrorSampleBuffer{ [] { GLuint ubo; glGenBuffers(1, &ubo); return ubo; }() },
  receiverModel{ std::make_unique<GgSimpleObj>(config.receiverModel, true) },
  drawMode{ DRAW_MIRROR }
{
#if defined(IMGUI_VERSION)
  //
  // ImGui の初期設定
  //

  // ファイルダイアログ (Native File Dialog Extended) を初期化する
  NFD_Init();

  // Dear ImGui の入力デバイス
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // キーボードコントロールを使う
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // ゲームパッドを使う

  // Dear ImGui のスタイル
  //ImGui::StyleColorsDark();                                 // 暗めのスタイル
  //ImGui::StyleColorsClassic();                              // 以前のスタイル

  // 日本語を表示できるメニューフォントを読み込む
  if (!ImGui::GetIO().Fonts->AddFontFromFileTTF(config.menuFont.c_str(), config.menuFontSize,
    nullptr, ImGui::GetIO().Fonts->GetGlyphRangesJapanese()))
  {
    // メニューフォントが読み込めなかったらエラーにする
    throw std::runtime_error("Cannot find any menu fonts.");
  }
#endif

  // 全体光源を初期化する
  setLight();

  // 投影光源データと姿勢を初期化する
  setIlluminantIntensity();
  setIlluminantPose();

  // 鏡の材質のユニフォームバッファオブジェクトを作成する
  glBindBuffer(GL_UNIFORM_BUFFER, mirrorMaterialBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(GgSimpleShader::Material), nullptr, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // 鏡の材質と姿勢を初期化する
  setMirrorMaterial();
  setMirrorPose();

  // 鏡の標本点を生成する
  generateMirrorSample(MAX_MIRROR_SAMPLES);

  // 受光面の姿勢を初期化する
  setReceiverPose();
}

//
// デストラクタ
//
Menu::~Menu()
{
  // 鏡の材質のユニフォームバッファオブジェクトを削除する
  glDeleteBuffers(1, &mirrorMaterialBuffer);

  // 鏡の標本点のユニフォームバッファオブジェクトを削除する
  glDeleteBuffers(1, &mirrorSampleBuffer);

  // 鏡の高さマップのテクスチャを削除する
  glDeleteTextures(1, &mirrorHeightMap);

  // 投影光源マップのテクスチャを削除する
  glDeleteTextures(1, &illuminantMap);

  // Native File Dialog Extended を終了する
  NFD_Quit();
}

//
// ファイルパスを取得する
//
bool Menu::getFilePath(std::string& path, const nfdfilteritem_t* filter)
{
  // ファイルダイアログから得るパス
  nfdchar_t* filepath{ nullptr };

  // ファイルダイアログを開く
  if (NFD_OpenDialog(&filepath, filter, 1, nullptr) == NFD_OKAY)
  {
    path = TCharToUtf8(filepath);
    return true;
  }

  return false;
}

//
// 設定ファイルを読み込む
//
void Menu::loadConfig()
{
  // ファイルダイアログから得るパス
  nfdchar_t* filepath;

  // ファイルダイアログを開く
  if (NFD_OpenDialog(&filepath, jsonFilter, 1, NULL) == NFD_OKAY)
  {
    // 現在の構成を構成ファイルの内容にする
    if (!settings.load(filepath))
    {
      // 読み込めなかった
      errorMessage = u8"設定ファイルが読み込めません";
    }

    // ファイルパスの取り出しに使ったメモリを開放する
    NFD_FreePath(filepath);
  }
}

//
// 設定ファイルを保存する
//
void Menu::saveConfig() const
{
  // ファイルダイアログから得るパス
  nfdchar_t* filepath;

  // ファイルダイアログを開く
  if (NFD_SaveDialog(&filepath, jsonFilter, 1, NULL, "*.json") == NFD_OKAY)
  {
    // 現在の設定で構成を更新する
    const_cast<Config&>(defaults) = settings;

    // 現在の構成を保存する
    if (!settings.save(filepath))
    {
      // 保存できなかった
      errorMessage = u8"設定ファイルが保存できません";
    }

    // ファイルパスの取り出しに使ったメモリを開放する
    NFD_FreePath(filepath);
  }
}

//
// 鏡の高さマップを読み込む
//
void Menu::loadMirrorHeightMap()
{
  // 鏡の高さマップのファイル名
  std::string path{ settings.illuminantMap };

  // ファイルダイアログから得るパス
  if (getFilePath(path, imageFilter))
  {
    // 鏡の高さマップを読み込んでテクスチャを作成する
    const auto height{ loadImage(path) };

    // 読み込みに成功したら
    if (height != 0)
    {
      // ファイル名を保存する
      settings.mirrorHeightMap = path;

      // それまで使っていたテクスチャを破棄して
      glDeleteTextures(1, &mirrorHeightMap);

      // テクスチャ名を保存する
      mirrorHeightMap = height;
    }
    else
    {
      // 読み込みに失敗したらエラーにする
      errorMessage = u8"高さマップが読み込めません";
    }
  }
}

//
// 投影光源マップを読み込む
//
void Menu::loadIlluminantMap()
{
  // 投影光源マップのファイル名
  std::string path{ settings.illuminantMap };

  // ファイルダイアログから得るパス
  if (getFilePath(path, imageFilter))
  {
    // 投影光源マップを読み込んでテクスチャを作成する
    const auto color{ loadImage(path) };

    // 読み込みに成功したら
    if (color != 0)
    {
      // ファイル名を保存する
      settings.illuminantMap = path;

      // それまで使っていたテクスチャを破棄して
      glDeleteTextures(1, &illuminantMap);

      // テクスチャ名を保存する
      illuminantMap = color;
    }
    else
    {
      // 読み込みに失敗したらエラーにする
      errorMessage = u8"光源マップが読み込めません";
    }
  }
}

//
// 受光面の形状ファイルを読み込む
//
void Menu::loadReceiverModel()
{
  // ファイルダイアログから得るパス
  nfdchar_t* filepath;

  // ファイルダイアログを開く
  if (NFD_OpenDialog(&filepath, shapeFilter, 1, NULL) == NFD_OKAY)
  {
    auto path{ TCharToUtf8(filepath) };
    GgSimpleObj object(filepath, true);
    if (object)
    {
      // 受光面の形状ファイル名を保存する
      settings.receiverModel = path;

      // 受光面のモデルを読み込む
      receiverModel = std::make_unique<GgSimpleObj>(object);
    }
    else
    {
      // 読み込めなかったらエラーにする
      errorMessage = u8"形状ファイルが読み込めません";
    }

    // ファイルパスの取り出しに使ったメモリを開放する
    NFD_FreePath(filepath);
  }
}

//
// 全体光源の強度と位置を設定する
//
void Menu::setLight()
{
  const GgSimpleShader::Light lightData
  {
    settings.lightColor * settings.lightIntensity * settings.lightAmbient,
    settings.lightColor * settings.lightIntensity,
    settings.lightColor * settings.lightIntensity,
    settings.lightPosition
  };
  light->load(lightData);
}

//
// 投影光源データを設定する
//
void Menu::setIlluminantIntensity()
{
  const GgSimpleShader::Light illuminantData
  {
    settings.illuminantColor * settings.illuminantIntensity * settings.illuminantAmbient,
    settings.illuminantColor * settings.illuminantIntensity,
    settings.illuminantColor * settings.illuminantIntensity,
    settings.illuminantPosition
  };
  illuminant->load(illuminantData);
}

//
// 投影光源の姿勢を設定する
//
void Menu::setIlluminantPose()
{
  // 投影光源の姿勢を設定する
  setPose(illuminantPose, settings.illuminantPosition, settings.illuminantTarget);
}

//
// 鏡の材質を設定する
//
void Menu::setMirrorMaterial()
{
  glBindBuffer(GL_UNIFORM_BUFFER, mirrorMaterialBuffer);
  auto* const material{ static_cast<GgSimpleShader::Material*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY)) };
  material->ambient = material->diffuse = settings.mirrorMaterialDiffuse;
  material->specular = settings.mirrorMaterialSpecular;
  material->shininess = settings.mirrorMaterialShininess;
  glUnmapBuffer(GL_UNIFORM_BUFFER);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//
// 鏡の標本点を生成する
//
void Menu::generateMirrorSample(int samples)
{
  // シェーダストレージバッファオブジェクトを作成する
  glBindBuffer(GL_UNIFORM_BUFFER, mirrorSampleBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(std::array<GLfloat, 2>) * samples, nullptr, GL_STATIC_DRAW);

  // 擬似乱数生成器
  std::mt19937 engine(11);

  // [-1.0f, 1.0f) の範囲の一様乱数
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  
  // 鏡の標本点を生成する
  auto* const sample{ static_cast<std::array<GLfloat, 4>*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY)) };
  for (int i = 0; i < samples;)
  {
    // 一様乱数を生成する
    const auto u{ dist(engine) };
    const auto v{ dist(engine) };

    // 単位円の内部に入っていなかったらやり直す
    if (u * u + v * v >= 1.0f) continue;

    // 標本点を格納する
    sample[i++] = { u, v, 0.0f, 1.0f };
  }
  glUnmapBuffer(GL_UNIFORM_BUFFER);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//
// 鏡の姿勢を設定する
//
void Menu::setMirrorPose()
{
  // 鏡の姿勢を設定する
  setPose(mirrorPose, settings.mirrorPosition, settings.mirrorTarget);
}

//
// 受光面の姿勢を設定する
//
void Menu::setReceiverPose()
{
  // 受光面の姿勢を設定する
  const auto& scale{ settings.receiverOrientation[3] };
  const auto rotation{ ggEulerQuaternion(settings.receiverOrientation) };
  receiverPose = ggTranslate(settings.receiverPosition) * rotation.getMatrix()
    * ggScale(scale, scale, scale);

  // 受光面の視界を設定する
  const auto& translate{ settings.receiverPosition };
  receiverView = rotation.getMatrix().transpose()
    * ggTranslate(-translate[0], -translate[1], -translate[2], translate[3]);
}

//
// メニューを描画する
//
void Menu::draw()
{
#if defined(IMGUI_VERSION)
  //
  // ImGui によるユーザインタフェース
  //

  // メニューの表示位置を決定する
  ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(294, 848), ImGuiCond_Once);

  // メニュー表示開始
  ImGui::Begin(u8"コントロールパネル");

  // 全体光源
  ImGui::SeparatorText(u8"全体光源");
  if (ImGui::DragFloat3(u8"位置##全体", settings.lightPosition.data(), 0.01f, -10.0f, 10.0f, "%.2f"))
    light->loadPosition(settings.lightPosition);
  if (ImGui::ColorEdit3(u8"色##全体", settings.lightColor.data(), ImGuiColorEditFlags_Float))
    setLight();
  if (ImGui::SliderFloat(u8"強度##全体", &settings.lightIntensity, 0.0f, 10.0f, "%.2f"))
    setLight();
  if (ImGui::SliderFloat(u8"環境光成分##全体", &settings.lightAmbient, 0.0f, 1.0f, "%.2f"))
    light->loadAmbient(settings.lightColor * settings.lightIntensity * settings.lightAmbient);
  if (ImGui::Button(u8"位置を初期化##全体"))
  {
    settings.lightPosition = defaults.lightPosition;
    light->loadPosition(settings.lightPosition);
  }
  ImGui::SameLine();
  if (ImGui::Button(u8"強度を初期化##全体"))
  {
    settings.lightColor = defaults.lightColor;
    settings.lightIntensity = defaults.lightIntensity;
    settings.lightAmbient = defaults.lightAmbient;
    setLight();
  }

  // 投影光源
  ImGui::SeparatorText(u8"投影光源");
  if (ImGui::DragFloat3(u8"位置##投影", settings.illuminantPosition.data(), 0.01f, -10.0f, 10.0f, "%.2f"))
    setIlluminantPose();
  if (ImGui::DragFloat3(u8"目標##投影", settings.illuminantTarget.data(), 0.01f, -10.0f, 10.0f, "%.2f"))
    setIlluminantPose();
#ifdef USE_ILLUMINANT_COLOR
  if (ImGui::ColorEdit3(u8"色##投影", settings.illuminantColor.data(), ImGuiColorEditFlags_Float))
    setIlluminantIntensity();
  if (ImGui::SliderFloat(u8"強度##投影", &settings.illuminantIntensity, 0.0f, 10.0f, "%.2f"))
    setIlluminantIntensity();
  if (ImGui::SliderFloat(u8"環境光成分##投影", &settings.illuminantAmbient, 0.0f, 1.0f, "%.2f"))
    illuminant->loadAmbient(settings.illuminantColor * settings.illuminantIntensity * settings.illuminantAmbient);
  ImGui::SliderFloat(u8"広がり##投影", &settings.illuminantSpread, 0.0f, 180.0f, "%.2f");
#endif
  if (ImGui::Button(u8"姿勢を初期化##投影"))
  {
    settings.illuminantPosition = defaults.illuminantPosition;
    settings.illuminantTarget = defaults.illuminantTarget;
    setIlluminantPose();
  }
  ImGui::SameLine();
#ifdef USE_ILLUMINANT_COLOR
  if (ImGui::Button(u8"強度を初期化##投影"))
  {
    settings.illuminantColor = defaults.illuminantColor;
    settings.illuminantIntensity = defaults.illuminantIntensity;
    settings.illuminantAmbient = defaults.illuminantAmbient;
    settings.illuminantSpread = defaults.illuminantSpread;
    setIlluminantIntensity();
  }
  ImGui::SameLine();
#endif
  if (ImGui::Button(u8"光源マップ##投影"))
    loadIlluminantMap();

  // 鏡
  ImGui::SeparatorText(u8"鏡");
  if (ImGui::DragFloat3(u8"位置##鏡", settings.mirrorPosition.data(), 0.01f, -10.0f, 10.0f, "%.2f"))
    setMirrorPose();
  if (ImGui::DragFloat3(u8"目標##鏡", settings.mirrorTarget.data(), 0.01f, -10.0f, 10.0f, "%.2f"))
    setMirrorPose();
  if (ImGui::ColorEdit3(u8"拡散反射係数", settings.mirrorMaterialDiffuse.data(), ImGuiColorEditFlags_Float))
    setMirrorMaterial();
  if (ImGui::ColorEdit3(u8"鏡面反射係数", settings.mirrorMaterialSpecular.data(), ImGuiColorEditFlags_Float))
    setMirrorMaterial();
  if (ImGui::SliderFloat(u8"輝き係数", &settings.mirrorMaterialShininess, 0.0f, 200.0f, "%.2f"))
    setMirrorMaterial();
  ImGui::SliderFloat(u8"高さスケール##鏡", &settings.mirrorHeightScale, -1.0f, 1.0f, "%.3f");
  ImGui::SliderInt(u8"標本点数##鏡", &settings.mirrorSampleCount, 1, MAX_MIRROR_SAMPLES);
  if (ImGui::Button(u8"姿勢を初期化##鏡"))
  {
    settings.mirrorPosition = defaults.mirrorPosition;
    settings.mirrorTarget = defaults.mirrorTarget;
    setMirrorPose();
  }
  ImGui::SameLine();
  if (ImGui::Button(u8"材質を初期化##鏡"))
  {
    settings.mirrorMaterialDiffuse = defaults.mirrorMaterialDiffuse;
    settings.mirrorMaterialSpecular = defaults.mirrorMaterialSpecular;
    settings.mirrorMaterialShininess = defaults.mirrorMaterialShininess;
    settings.mirrorHeightScale = defaults.mirrorHeightScale;
    setMirrorMaterial();
  }
  ImGui::SameLine();
  if (ImGui::Button(u8"高さマップ##鏡"))
    loadMirrorHeightMap();

  // 受光面
  ImGui::SeparatorText(u8"受光面");
  if (ImGui::DragFloat3(u8"位置##受光面", settings.receiverPosition.data(), 0.01f, -10.0f, 10.0f, "%.2f"))
    setReceiverPose();
  GgVector orientation{ settings.receiverOrientation * 57.2957795f };
  if (ImGui::DragFloat3(u8"回転##受光面", orientation.data(), 0.01f, -180.0f, 180.0f, "%.2f"))
  {
    settings.receiverOrientation = orientation * 0.0174532925f;
    setReceiverPose();
  }
  if (ImGui::Button(u8"姿勢を初期化##受光面"))
  {
    settings.receiverModel = defaults.receiverModel;
    settings.receiverPosition = defaults.receiverPosition;
    settings.receiverOrientation = defaults.receiverOrientation;
    setReceiverPose();
  }
  ImGui::SameLine();
  if (ImGui::Button(u8"形状ファイル##受光面"))
    loadReceiverModel();

  // 描画モード
  ImGui::SeparatorText(u8"描画モード");
  if (ImGui::RadioButton(u8"鏡", drawMode == DRAW_MIRROR)) drawMode = DRAW_MIRROR;
  ImGui::SameLine();
  if (ImGui::RadioButton(u8"受光面", drawMode == DRAW_RECEIVER)) drawMode = DRAW_RECEIVER;
  ImGui::SameLine();
  ImGui::Text(u8"(%.1f fps)", ImGui::GetIO().Framerate);

  // 設定ファイル
  ImGui::SeparatorText(u8"設定ファイル");
  if (ImGui::Button(u8"読み込み")) loadConfig();
  ImGui::SameLine();
  if (ImGui::Button(u8"書き出し")) saveConfig();

  // エラーメッセージが設定されていたら
  if (errorMessage)
  {
    // ウィンドウの位置・サイズとタイトル
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(240, 92), ImGuiCond_Always);

    // ウィンドウを表示するとき true
    bool status{ true };

    // エラーメッセージウィンドウを表示する
    ImGui::Begin(u8"エラー", &status);

    // エラーメッセージの表示
    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.0f, 1.0f), "%s", errorMessage);

    // クローズボックスか「閉じる」ボタンをクリックしたら
    if (!status || ImGui::Button(u8"閉じる"))
    {
      // エラーメッセージを消去する
      errorMessage = nullptr;
    }
    ImGui::End();
  }

  // メニュー表示終了
  ImGui::End();
#endif
}
