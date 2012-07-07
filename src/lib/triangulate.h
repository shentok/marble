//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free
// Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Copyright 2005       Christopher Nelson
// Copyright 2012       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef TRIANGULATE_H

#define TRIANGULATE_H

#include <QtCore/QVector>

namespace Marble {

class GeoDataCoordinates;
class GeoDataLinearRing;

class Triangulate
{
public:
  // triangulate a contour/polygon, places results in STL vector
  // as series of triangles.
  static QVector<int> Process( const GeoDataLinearRing &polygon );

private:
  // compute area of a contour/polygon
  static qreal area( const GeoDataLinearRing &polygon );

  // decide if point P is inside triangle defined by A B C
  static bool isInsideTriangle( const GeoDataCoordinates &A, const GeoDataCoordinates &B,
                      const GeoDataCoordinates &C, const GeoDataCoordinates &P );

  static bool isEar( const GeoDataLinearRing &polygon, int u, int v, int w, int n, const QVector<int> &V );
};

}

#endif
