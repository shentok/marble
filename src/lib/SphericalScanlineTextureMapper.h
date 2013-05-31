//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Torsten Rahn     <tackat@kde.org>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_SPHERICALSCANLINETEXTUREMAPPER_H
#define MARBLE_SPHERICALSCANLINETEXTUREMAPPER_H


#include "TextureMapperInterface.h"

#include "MarbleGlobal.h"

#include <QtCore/QThreadPool>
#include <QtGui/QImage>


namespace Marble
{

class StackedTileLoader;


/*
 * @short Texture mapping onto a sphere
 *
 * This class provides a fast way to map textures onto a sphere
 * without making use of hardware acceleration. 
 *
 * @author Torsten Rahn <rahn@kde.org>
 */

class SphericalScanlineTextureMapper : public TextureMapperInterface
{
 public:
    QRect rect( const ViewportParams *viewport ) const;

    void mapTexture( QImage *canvasImage, StackedTileLoader *tileLoader, const ViewportParams *viewport, int tileZoomLevel, MapQuality mapQuality );

 private:
    class RenderJob;
    QThreadPool m_threadPool;
};

}

#endif
