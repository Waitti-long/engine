// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "flutter/fml/macros.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/texture.h"
#include "impeller/typographer/font_glyph_pair.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      A texture containing the bitmap representation of glyphs in
///             different fonts along with the ability to query the location of
///             specific font glyphs within the texture.
///
class GlyphAtlas {
 public:
  //----------------------------------------------------------------------------
  /// @brief      Describes how the glyphs are represented in the texture.
  enum class Type {
    //--------------------------------------------------------------------------
    /// The glyphs are represented at a fixed size in an 8-bit grayscale texture
    /// where the value of each pixel represents a signed-distance field that
    /// stores the glyph outlines.
    ///
    kSignedDistanceField,

    //--------------------------------------------------------------------------
    /// The glyphs are reprsented at their requested size using only an 8-bit
    /// alpha channel.
    ///
    kAlphaBitmap,

    //--------------------------------------------------------------------------
    /// The glyphs are reprsented at their requested size using N32 premul
    /// colors.
    ///
    kColorBitmap,
  };

  //----------------------------------------------------------------------------
  /// @brief      Create an empty glyph atlas.
  ///
  /// @param[in]  type  How the glyphs are represented in the texture.
  ///
  explicit GlyphAtlas(Type type);

  ~GlyphAtlas();

  bool IsValid() const;

  //----------------------------------------------------------------------------
  /// @brief      Describes how the glyphs are represented in the texture.
  ///
  Type GetType() const;

  //----------------------------------------------------------------------------
  /// @brief      Set the texture for the glyph atlas.
  ///
  /// @param[in]  texture  The texture
  ///
  void SetTexture(std::shared_ptr<Texture> texture);

  //----------------------------------------------------------------------------
  /// @brief      Get the texture for the glyph atlas.
  ///
  /// @return     The texture.
  ///
  const std::shared_ptr<Texture>& GetTexture() const;

  //----------------------------------------------------------------------------
  /// @brief      Record the location of a specific font-glyph pair within the
  ///             atlas.
  ///
  /// @param[in]  pair  The font-glyph pair
  /// @param[in]  rect  The rectangle
  ///
  void AddTypefaceGlyphPosition(const FontGlyphPair& pair, Rect rect);

  //----------------------------------------------------------------------------
  /// @brief      Get the number of unique font-glyph pairs in this atlas.
  ///
  /// @return     The glyph count.
  ///
  size_t GetGlyphCount() const;

  //----------------------------------------------------------------------------
  /// @brief      Iterate of all the glyphs along with their locations in the
  ///             atlas.
  ///
  /// @param[in]  iterator  The iterator. Return `false` from the iterator to
  ///                       stop iterating.
  ///
  /// @return     The number of glyphs iterated over.
  ///
  size_t IterateGlyphs(
      const std::function<bool(const FontGlyphPair& pair, const Rect& rect)>&
          iterator) const;

  //----------------------------------------------------------------------------
  /// @brief      Find the location of a specific font-glyph pair in the atlas.
  ///
  /// @param[in]  pair  The font-glyph pair
  ///
  /// @return     The location of the font-glyph pair in the atlas.
  ///             `std::nullopt` of the pair in not in the atlas.
  ///
  std::optional<Rect> FindFontGlyphPosition(const FontGlyphPair& pair) const;

 private:
  const Type type_;
  std::shared_ptr<Texture> texture_;

  std::unordered_map<FontGlyphPair,
                     Rect,
                     FontGlyphPair::Hash,
                     FontGlyphPair::Equal>
      positions_;

  FML_DISALLOW_COPY_AND_ASSIGN(GlyphAtlas);
};

}  // namespace impeller
