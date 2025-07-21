//
// ゲームグラフィックス特論宿題アプリケーション
//

// 宿題用補助プログラムのラッパー
#include "GgApp.h"

// プロジェクト名
#if !defined(PROJECT_NAME)
#  define PROJECT_NAME "makyoh"
#endif

// 構成ファイル名
#if !defined(CONFIG_FILE)
#  define CONFIG_FILE PROJECT_NAME "_config.json"
#endif

// 構成データ
#include "Config.h"

// メニューの描画
#include "Menu.h"

// 矩形オブジェクト
#include "Rect.h"


// 鏡の材質のユニフォームバッファオブジェクトの結合ポイント
constexpr GLuint mirrorMaterialBindingPoint{ 2 };

// 鏡の標本点のユニフォームバッファオブジェクトの結合ポイント
constexpr GLuint mirrorSampleBindingPoint{ 3 };

//
// アプリケーション本体
//
int GgApp::main(int argc, const char* const* argv)
{
  // 設定を読み込む
  const Config config{ CONFIG_FILE };

  // ウィンドウを作成する
  Window window{ PROJECT_NAME, config.getWidth(), config.getHeight() };

  // メニューを初期化する
  Menu menu{ config };

  // 受光面のシェーダ
  const GgSimpleShader receiverShader{ "receiver.vert", "receiver.frag" };

  // 鏡の材質のユニフォームバッファオブジェクトの結合ポイントを設定する
  const auto receiverMirrorMaterialIndex = glGetUniformBlockIndex(receiverShader.get(), "Mirror");
  glUniformBlockBinding(receiverShader.get(), receiverMirrorMaterialIndex, mirrorMaterialBindingPoint);

  // 鏡の標本点のユニフォームバッファオブジェクトの結合ポイントを設定する
  const auto receiverMirrorSampleIndex = glGetUniformBlockIndex(receiverShader.get(), "Sample");
  glUniformBlockBinding(receiverShader.get(), receiverMirrorSampleIndex, mirrorSampleBindingPoint);

  // 鏡の標本点数の場所
  const auto receiverCountLoc{ glGetUniformLocation(receiverShader.get(), "samples") };

  // 鏡の高さマップのスケールの場所
  const auto receiverHeightScaleLoc{ glGetUniformLocation(receiverShader.get(), "scale") };

  // 鏡の高さマップのテクスチャのサンプラの場所
  const auto receiverHeightLoc{ glGetUniformLocation(receiverShader.get(), "height") };

  // 投影光源マップのテクスチャのサンプラの場所
  const auto receiverColorLoc{ glGetUniformLocation(receiverShader.get(), "color") };

  // 鏡の姿勢行列の場所
  const auto receiverMmLoc{ glGetUniformLocation(receiverShader.get(), "mm") };

  // 投影光源の姿勢行列の場所
  const auto receiverMlLoc{ glGetUniformLocation(receiverShader.get(), "ml") };

  // 鏡の矩形のオブジェクト
  const Rect mirror;

  // 鏡のシェーダ
  const GgSimpleShader mirrorShader{ "mirror.vert", "mirror.frag" };

  // 鏡の材質のユニフォームバッファオブジェクトの結合ポイントを設定する
  const auto mirrorMaterialIndex = glGetUniformBlockIndex(mirrorShader.get(), "Mirror");
  glUniformBlockBinding(mirrorShader.get(), mirrorMaterialIndex, mirrorMaterialBindingPoint);

  // 鏡の高さマップのスケールの場所
  const auto mirrorHeightScaleLoc{ glGetUniformLocation(mirrorShader.get(), "scale") };

  // 鏡の高さマップのテクスチャのサンプラの場所
  const auto mirrorHeightLoc{ glGetUniformLocation(mirrorShader.get(), "height") };

  // 投影光源マップのテクスチャのサンプラの場所
  const auto mirrorColorLoc{ glGetUniformLocation(mirrorShader.get(), "color") };

  // 投影光源の姿勢行列の場所
  const auto mirrorMlLoc{ glGetUniformLocation(mirrorShader.get(), "ml") };

  // 第３者視点の視線方向
  const auto eyePose{ ggLookat(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f) };

  // 背景色を設定する
  glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

  // 隠面消去処理を設定する
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // メニューを表示する
    menu.draw();

    // 鏡の高さマップを読み込む
    const auto height{ menu.getHeightMap() };

    // 投影光源マップを読み込む
    const auto color{ menu.getIlluminantMap() };

    // マウス操作によるシーン全体の視点移動
    const auto& mv{ window.getTranslationMatrix(1) * window.getRotationMatrix(0) };

    // 投影変換行列を設定する
    const GgMatrix&& mp{ ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f) };

    // 鏡の材質を設定する
    menu.bindMirrorMaterial(mirrorMaterialBindingPoint);

    // 鏡の高さマップを設定する
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, height);

    // 投影光源マップを設定する
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, color);

    // 描画
    if (menu.getDrawMode() == Menu::DRAW_MIRROR)
    {
      // 鏡だけを描画する
      mirrorShader.use(mp, menu.getReceiverView() * menu.getMirrorPose(), menu.getLight());
      glUniform1f(mirrorHeightScaleLoc, menu.getMirrorHeightScale());
      glUniform1i(mirrorHeightLoc, 0);
      glUniform1i(mirrorColorLoc, 1);
      glUniformMatrix4fv(mirrorMlLoc, 1, GL_FALSE, menu.getIlluminantPose().get());
      mirror.draw();
    }
    else if (menu.getDrawMode() == Menu::DRAW_RECEIVER)
    {
      // 鏡の標本点を設定する
      menu.bindMirrorSample(mirrorSampleBindingPoint);

      // 受光面だけを描画する
      receiverShader.use(mp, eyePose * menu.getReceiverPose() * mv, menu.getLight());
      glUniform1i(receiverCountLoc, menu.getMirrorSampleCount());
      glUniform1f(receiverHeightScaleLoc, menu.getMirrorHeightScale());
      glUniform1i(receiverHeightLoc, 0);
      glUniform1i(receiverColorLoc, 1);
      glUniformMatrix4fv(receiverMmLoc, 1, GL_FALSE, (eyePose * menu.getMirrorPose() * mv).get());
      glUniformMatrix4fv(receiverMlLoc, 1, GL_FALSE, (eyePose * menu.getIlluminantPose() * mv).get());
      menu.getReceiverModel().draw();
    }

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return 0;
}
