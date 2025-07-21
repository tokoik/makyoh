///
/// 矩形クラスの実装
///
/// @file
/// @author Kohe Tokoi
/// @date August 12, 2025
///
#include "Rect.h"

///
/// コンストラクタ
///
Rect::Rect() :
  vao{ [] { GLuint vao; glGenVertexArrays(1, &vao); return vao; } () }
{
}

///
/// デストラクタ
///
Rect::~Rect()
{
  glDeleteVertexArrays(1, &vao);
}

///
/// 描画する
///
void Rect::draw() const
{
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}
