#ifdef _MSC_VER
#pragma warning(disable: 4244)
#endif

#include "CGUIImageButton.h"

namespace irr {
namespace gui {

void Draw2DImageRotation(video::IVideoDriver* driver, video::ITexture* image, core::rect<s32> sourceRect,
                         core::position2d<s32> position, core::position2d<s32> rotationPoint, f32 rotation, core::vector2df scale, bool useAlphaChannel, video::SColor color) {
	irr::video::SMaterial material;
	irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION);
	driver->setTransform(irr::video::ETS_PROJECTION, irr::core::matrix4());
	irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW);
	driver->setTransform(irr::video::ETS_VIEW, irr::core::matrix4());
	irr::core::vector2df corner[4];
	corner[0] = irr::core::vector2df(position.X, position.Y);
	corner[1] = irr::core::vector2df(position.X + sourceRect.getWidth() * scale.X, position.Y);
	corner[2] = irr::core::vector2df(position.X, position.Y + sourceRect.getHeight() * scale.Y);
	corner[3] = irr::core::vector2df(position.X + sourceRect.getWidth() * scale.X, position.Y + sourceRect.getHeight() * scale.Y);
	if (rotation != 0.0f) {
		for (int x = 0; x < 4; x++)
			corner[x].rotateBy(rotation, irr::core::vector2df(rotationPoint.X, rotationPoint.Y));
	}
	irr::core::vector2df uvCorner[4];
	uvCorner[0] = irr::core::vector2df(sourceRect.UpperLeftCorner.X, sourceRect.UpperLeftCorner.Y);
	uvCorner[1] = irr::core::vector2df(sourceRect.LowerRightCorner.X, sourceRect.UpperLeftCorner.Y);
	uvCorner[2] = irr::core::vector2df(sourceRect.UpperLeftCorner.X, sourceRect.LowerRightCorner.Y);
	uvCorner[3] = irr::core::vector2df(sourceRect.LowerRightCorner.X, sourceRect.LowerRightCorner.Y);
	for (int x = 0; x < 4; x++) {
		float uvX = uvCorner[x].X / (float)image->getOriginalSize().Width;
		float uvY = uvCorner[x].Y / (float)image->getOriginalSize().Height;
		uvCorner[x] = irr::core::vector2df(uvX, uvY);
	}
	irr::video::S3DVertex vertices[4];
	irr::u16 indices[6] = { 0, 1, 2, 3, 2, 1 };
	float screenWidth = driver->getScreenSize().Width;
	float screenHeight = driver->getScreenSize().Height;
	for (int x = 0; x < 4; x++) {
		float screenPosX = ((corner[x].X / screenWidth) - 0.5f) * 2.0f;
		float screenPosY = ((corner[x].Y / screenHeight) - 0.5f) * -2.0f;
		vertices[x].Pos = irr::core::vector3df(screenPosX, screenPosY, 1);
		vertices[x].TCoords = uvCorner[x];
		vertices[x].Color = color;
	}
	material.Lighting = false;
	material.ZWriteEnable = false;
	material.TextureLayer[0].Texture = image;
	if (useAlphaChannel)
		material.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	else material.MaterialType = irr::video::EMT_SOLID;
	driver->setMaterial(material);
	driver->drawIndexedTriangleList(&vertices[0], 4, &indices[0], 2);
	driver->setTransform(irr::video::ETS_PROJECTION, oldProjMat);
	driver->setTransform(irr::video::ETS_VIEW, oldViewMat);
}
void Draw2DImageQuad(video::IVideoDriver* driver, video::ITexture* image, core::rect<s32> sourceRect,
                         core::position2d<s32> corner[4], bool useAlphaChannel, video::SColor color) {
	irr::video::SMaterial material;
	irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION);
	driver->setTransform(irr::video::ETS_PROJECTION, irr::core::matrix4());
	irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW);
	driver->setTransform(irr::video::ETS_VIEW, irr::core::matrix4());
	irr::core::vector2df uvCorner[4];
	uvCorner[0] = irr::core::vector2df(sourceRect.UpperLeftCorner.X, sourceRect.UpperLeftCorner.Y);
	uvCorner[1] = irr::core::vector2df(sourceRect.LowerRightCorner.X, sourceRect.UpperLeftCorner.Y);
	uvCorner[2] = irr::core::vector2df(sourceRect.UpperLeftCorner.X, sourceRect.LowerRightCorner.Y);
	uvCorner[3] = irr::core::vector2df(sourceRect.LowerRightCorner.X, sourceRect.LowerRightCorner.Y);
	for (int x = 0; x < 4; x++) {
		float uvX = uvCorner[x].X / (float)image->getOriginalSize().Width;
		float uvY = uvCorner[x].Y / (float)image->getOriginalSize().Height;
		uvCorner[x] = irr::core::vector2df(uvX, uvY);
	}
	irr::video::S3DVertex vertices[4];
	irr::u16 indices[6] = { 0, 1, 2, 3, 2, 1 };
	float screenWidth = driver->getScreenSize().Width;
	float screenHeight = driver->getScreenSize().Height;
	for (int x = 0; x < 4; x++) {
		float screenPosX = ((corner[x].X / screenWidth) - 0.5f) * 2.0f;
		float screenPosY = ((corner[x].Y / screenHeight) - 0.5f) * -2.0f;
		vertices[x].Pos = irr::core::vector3df(screenPosX, screenPosY, 1);
		vertices[x].TCoords = uvCorner[x];
		vertices[x].Color = color;
	}
	material.Lighting = false;
	material.ZWriteEnable = false;
	material.TextureLayer[0].Texture = image;
	if (useAlphaChannel)
		material.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	else material.MaterialType = irr::video::EMT_SOLID;
	driver->setMaterial(material);
	driver->drawIndexedTriangleList(&vertices[0], 4, &indices[0], 2);
	driver->setTransform(irr::video::ETS_PROJECTION, oldProjMat);
	driver->setTransform(irr::video::ETS_VIEW, oldViewMat);
}
CGUIImageButton::CGUIImageButton(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
	: CGUIButton(environment, parent, id, rectangle) {
	isDrawImage = true;
	imageRotation = 0.0f;
	imageScale = core::vector2df(1.0f, 1.0f);
	Cover = NULL;
}
CGUIImageButton* CGUIImageButton::addImageButton(IGUIEnvironment *env, const core::rect<s32>& rectangle, IGUIElement* parent, s32 id) {
	CGUIImageButton* button = new CGUIImageButton(env, parent ? parent : 0, id, rectangle);
	button->drop();
	return button;
}

void CGUIImageButton::setCover(video::ITexture* cover)
{
	Cover = cover;
}

void CGUIImageButton::draw() {
	if (!IsVisible)
		return;
	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();
	core::position2di center = AbsoluteRect.getCenter();
	core::position2di pos = center;
	pos.X -= (s32)(ImageRect.getWidth() * imageScale.X * 0.5f);
	pos.Y -= (s32)(ImageRect.getHeight() * imageScale.Y * 0.5f);
	if(Pressed) {
		pos.X += 1;
		pos.Y += 1;
		center.X += 1;
		center.Y += 1;
		if (DrawBorder)
			skin->draw3DButtonPanePressed(this, AbsoluteRect, &AbsoluteClippingRect);
	} else {
		if (DrawBorder)
			skin->draw3DButtonPaneStandard(this, AbsoluteRect, &AbsoluteClippingRect);
	}
	if(Image && isDrawImage)
		//change
		//driver->draw2DImage(Image,AbsoluteRect,ImageRect);
	{
		f32 ImageX=(imageRotation==90.0 || imageRotation==270.0) ?ImageRect.LowerRightCorner.Y :ImageRect.LowerRightCorner.X;
		f32 ImageY=(imageRotation==90.0 || imageRotation==270.0) ?ImageRect.LowerRightCorner.X :ImageRect.LowerRightCorner.Y;
		f32 ImageScale=irr::core::min_((AbsoluteRect.LowerRightCorner.X-AbsoluteRect.UpperLeftCorner.X)/ImageX,(AbsoluteRect.LowerRightCorner.Y-AbsoluteRect.UpperLeftCorner.Y)/ImageY);
		imageScale=irr::core::vector2df( ImageScale,ImageScale );
		irr::gui::Draw2DImageRotation(driver, Image, ImageRect, pos, center, imageRotation, imageScale);
		if(Cover)
			irr::gui::Draw2DImageRotation(driver, Cover, ImageRect, pos, center, imageRotation, imageScale);
	}
	IGUIElement::draw();
}
void CGUIImageButton::setDrawImage(bool b) {
	isDrawImage = b;
}
void CGUIImageButton::setImageRotation(f32 r) {
	imageRotation = r;
}
void CGUIImageButton::setImageScale(core::vector2df s) {
	imageScale = s;
}

IGUIFont* CGUIImageButton::getOverrideFont( void ) const
{
	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return NULL;
	return skin->getFont();
}

IGUIFont* CGUIImageButton::getActiveFont() const
{
	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return NULL;
	return skin->getFont();
}

}
}