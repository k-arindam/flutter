// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display_list/dl_sampling_options.h"
#include "display_list/dl_types.h"
#include "display_list/effects/dl_color_filter.h"
#include "display_list/effects/image_filters/dl_matrix_image_filter.h"
#include "display_list/geometry/dl_geometry_types.h"
#include "flutter/impeller/display_list/aiks_unittests.h"

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_builder.h"
#include "flutter/display_list/dl_color.h"
#include "flutter/display_list/dl_paint.h"
#include "flutter/testing/testing.h"
#include "impeller/core/formats.h"
#include "impeller/display_list/dl_atlas_geometry.h"
#include "impeller/display_list/dl_image_impeller.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/scalar.h"

namespace impeller {
namespace testing {

using namespace flutter;

namespace {
RSTransform MakeTranslation(Scalar tx, Scalar ty) {
  return RSTransform::Make({tx, ty}, 1, DlDegrees(0));
}

std::tuple<std::vector<DlRect>,       //
           std::vector<RSTransform>,  //
           sk_sp<DlImageImpeller>>
CreateTestData(const AiksTest* test) {
  // Draws the image as four squares stiched together.
  auto atlas =
      DlImageImpeller::Make(test->CreateTextureForFixture("bay_bridge.jpg"));
  auto size = atlas->impeller_texture()->GetSize();
  // Divide image into four quadrants.
  Scalar half_width = size.width / 2;
  Scalar half_height = size.height / 2;
  std::vector<DlRect> texture_coordinates = {
      DlRect::MakeLTRB(0, 0, half_width, half_height),
      DlRect::MakeLTRB(half_width, 0, size.width, half_height),
      DlRect::MakeLTRB(0, half_height, half_width, size.height),
      DlRect::MakeLTRB(half_width, half_height, size.width, size.height)};
  // Position quadrants adjacent to eachother.
  std::vector<RSTransform> transforms = {
      MakeTranslation(0, 0), MakeTranslation(half_width, 0),
      MakeTranslation(0, half_height),
      MakeTranslation(half_width, half_height)};
  return std::make_tuple(texture_coordinates, transforms, atlas);
}

}  // namespace

TEST_P(AiksTest, DrawAtlasNoColor) {
  DisplayListBuilder builder;
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  builder.Scale(GetContentScale().x, GetContentScale().y);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    /*colors=*/nullptr, /*count=*/4, DlBlendMode::kSrcOver,
                    DlImageSampling::kNearestNeighbor, nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawAtlasWithColorAdvanced) {
  DisplayListBuilder builder;
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  std::vector<DlColor> colors = {DlColor::kRed(), DlColor::kGreen(),
                                 DlColor::kBlue(), DlColor::kYellow()};

  builder.Scale(GetContentScale().x, GetContentScale().y);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    colors.data(), /*count=*/4, DlBlendMode::kModulate,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawAtlasWithColorSimple) {
  DisplayListBuilder builder;
  // Draws the image as four squares stiched together.
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  std::vector<DlColor> colors = {DlColor::kRed(), DlColor::kGreen(),
                                 DlColor::kBlue(), DlColor::kYellow()};

  builder.Scale(GetContentScale().x, GetContentScale().y);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    colors.data(), /*count=*/4, DlBlendMode::kSrcATop,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawAtlasWithOpacity) {
  DisplayListBuilder builder;
  // Draws the image as four squares stiched together slightly
  // opaque
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  DlPaint paint;
  paint.setAlpha(128);
  builder.Scale(GetContentScale().x, GetContentScale().y);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    /*colors=*/nullptr, 4, DlBlendMode::kSrcOver,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr,
                    &paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawAtlasNoColorFullSize) {
  auto atlas = DlImageImpeller::Make(CreateTextureForFixture("bay_bridge.jpg"));
  auto size = atlas->impeller_texture()->GetSize();
  std::vector<DlRect> texture_coordinates = {
      DlRect::MakeLTRB(0, 0, size.width, size.height)};
  std::vector<RSTransform> transforms = {MakeTranslation(0, 0)};

  DisplayListBuilder builder;
  builder.Scale(GetContentScale().x, GetContentScale().y);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    /*colors=*/nullptr, /*count=*/1, DlBlendMode::kSrcOver,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

// Regression test for https://github.com/flutter/flutter/issues/127374.
TEST_P(AiksTest, DrawAtlasAdvancedAndTransform) {
  DisplayListBuilder builder;
  // Draws the image as four squares stiched together.
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  builder.Scale(0.25, 0.25);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    /*colors=*/nullptr, /*count=*/4, DlBlendMode::kModulate,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

// Regression test for https://github.com/flutter/flutter/issues/127374.
TEST_P(AiksTest, DrawAtlasWithColorAdvancedAndTransform) {
  DisplayListBuilder builder;
  // Draws the image as four squares stiched together.
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);
  std::vector<DlColor> colors = {DlColor::kRed(), DlColor::kGreen(),
                                 DlColor::kBlue(), DlColor::kYellow()};

  builder.Scale(0.25, 0.25);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    colors.data(), /*count=*/4, DlBlendMode::kModulate,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawAtlasPlusWideGamut) {
  DisplayListBuilder builder;
  EXPECT_EQ(GetContext()->GetCapabilities()->GetDefaultColorFormat(),
            PixelFormat::kB10G10R10A10XR);

  // Draws the image as four squares stiched together.
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);
  std::vector<DlColor> colors = {DlColor::kRed(), DlColor::kGreen(),
                                 DlColor::kBlue(), DlColor::kYellow()};

  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    colors.data(), /*count=*/4, DlBlendMode::kPlus,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DlAtlasGeometryNoBlendRenamed) {
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  DlAtlasGeometry geom(atlas->impeller_texture(), transforms.data(),
                       texture_coordinates.data(), nullptr, transforms.size(),
                       BlendMode::kSrcOver, {}, std::nullopt);

  EXPECT_FALSE(geom.ShouldUseBlend());
  EXPECT_FALSE(geom.ShouldSkip());

  ContentContext context(GetContext(), nullptr);
  auto vertex_buffer =
      geom.CreateSimpleVertexBuffer(context.GetTransientsBuffer());

  EXPECT_EQ(vertex_buffer.index_type, IndexType::kNone);
  EXPECT_EQ(vertex_buffer.vertex_count, texture_coordinates.size() * 6);
}

TEST_P(AiksTest, DlAtlasGeometryBlend) {
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  std::vector<DlColor> colors;
  colors.reserve(texture_coordinates.size());
  for (auto i = 0u; i < texture_coordinates.size(); i++) {
    colors.push_back(DlColor::ARGB(0.5, 1, 1, 1));
  }
  DlAtlasGeometry geom(
      atlas->impeller_texture(), transforms.data(), texture_coordinates.data(),
      colors.data(), transforms.size(), BlendMode::kSrcOver, {}, std::nullopt);

  EXPECT_TRUE(geom.ShouldUseBlend());
  EXPECT_FALSE(geom.ShouldSkip());

  ContentContext context(GetContext(), nullptr);
  auto vertex_buffer =
      geom.CreateBlendVertexBuffer(context.GetTransientsBuffer());

  EXPECT_EQ(vertex_buffer.index_type, IndexType::kNone);
  EXPECT_EQ(vertex_buffer.vertex_count, texture_coordinates.size() * 6);
}

TEST_P(AiksTest, DlAtlasGeometryColorButNoBlend) {
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  std::vector<DlColor> colors;
  colors.reserve(texture_coordinates.size());
  for (auto i = 0u; i < texture_coordinates.size(); i++) {
    colors.push_back(DlColor::ARGB(0.5, 1, 1, 1));
  }
  DlAtlasGeometry geom(atlas->impeller_texture(), transforms.data(),
                       texture_coordinates.data(), colors.data(),
                       transforms.size(), BlendMode::kSrc, {}, std::nullopt);

  // Src blend mode means that colors would be ignored, even if provided.
  EXPECT_FALSE(geom.ShouldUseBlend());
  EXPECT_FALSE(geom.ShouldSkip());
}

TEST_P(AiksTest, DlAtlasGeometrySkip) {
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  std::vector<DlColor> colors;
  colors.reserve(texture_coordinates.size());
  for (auto i = 0u; i < texture_coordinates.size(); i++) {
    colors.push_back(DlColor::ARGB(0.5, 1, 1, 1));
  }
  DlAtlasGeometry geom(atlas->impeller_texture(), transforms.data(),
                       texture_coordinates.data(), colors.data(),
                       transforms.size(), BlendMode::kClear, {}, std::nullopt);
  EXPECT_TRUE(geom.ShouldSkip());
}

TEST_P(AiksTest, DrawImageRectWithBlendColorFilter) {
  sk_sp<DlImageImpeller> texture =
      DlImageImpeller::Make(CreateTextureForFixture("bay_bridge.jpg"));

  DisplayListBuilder builder;
  DlPaint paint = DlPaint().setColorFilter(DlColorFilter::MakeBlend(
      DlColor::kRed().withAlphaF(0.4), DlBlendMode::kSrcOver));

  DlMatrix filter_matrix = DlMatrix();
  auto filter = flutter::DlMatrixImageFilter(filter_matrix,
                                             flutter::DlImageSampling::kLinear);
  DlPaint paint_with_filter = paint;
  paint_with_filter.setImageFilter(&filter);

  // Compare porter-duff blend modes.
  builder.DrawPaint(DlPaint().setColor(DlColor::kWhite()));
  // Uses image filter to disable atlas conversion.
  builder.DrawImageRect(texture, DlRect::MakeSize(texture->GetSize()),
                        DlRect::MakeLTRB(0, 0, 500, 500), {},
                        &paint_with_filter);

  // Uses atlas conversion.
  builder.Translate(600, 0);
  builder.DrawImageRect(texture, DlRect::MakeSize(texture->GetSize()),
                        DlRect::MakeLTRB(0, 0, 500, 500), {}, &paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawImageRectWithMatrixColorFilter) {
  sk_sp<DlImageImpeller> texture =
      DlImageImpeller::Make(CreateTextureForFixture("bay_bridge.jpg"));

  DisplayListBuilder builder;
  static const constexpr ColorMatrix kColorInversion = {
      .array = {
          -1.0, 0,    0,    1.0, 0,  //
          0,    -1.0, 0,    1.0, 0,  //
          0,    0,    -1.0, 1.0, 0,  //
          1.0,  1.0,  1.0,  1.0, 0   //
      }};
  DlPaint paint = DlPaint().setColorFilter(
      DlColorFilter::MakeMatrix(kColorInversion.array));

  DlMatrix filter_matrix = DlMatrix();
  auto filter = flutter::DlMatrixImageFilter(filter_matrix,
                                             flutter::DlImageSampling::kLinear);
  DlPaint paint_with_filter = paint;
  paint_with_filter.setImageFilter(&filter);

  // Compare inverting color matrix filter.
  builder.DrawPaint(DlPaint().setColor(DlColor::kWhite()));
  // Uses image filter to disable atlas conversion.
  builder.DrawImageRect(texture, DlRect::MakeSize(texture->GetSize()),
                        DlRect::MakeLTRB(0, 0, 500, 500), {},
                        &paint_with_filter);

  // Uses atlas conversion.
  builder.Translate(600, 0);
  builder.DrawImageRect(texture, DlRect::MakeSize(texture->GetSize()),
                        DlRect::MakeLTRB(0, 0, 500, 500), {}, &paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, DrawAtlasWithColorBurn) {
  DisplayListBuilder builder;
  auto [texture_coordinates, transforms, atlas] = CreateTestData(this);

  std::vector<DlColor> colors = {DlColor::kDarkGrey(), DlColor::kBlack(),
                                 DlColor::kLightGrey(), DlColor::kWhite()};

  builder.Scale(GetContentScale().x, GetContentScale().y);
  builder.DrawAtlas(atlas, transforms.data(), texture_coordinates.data(),
                    colors.data(), /*count=*/4, DlBlendMode::kColorBurn,
                    DlImageSampling::kNearestNeighbor, /*cullRect=*/nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

}  // namespace testing
}  // namespace impeller
