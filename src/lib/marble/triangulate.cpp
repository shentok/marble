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

#include "triangulate.h"

#include "GeoDataCoordinates.h"
#include "GeoDataLinearRing.h"

namespace Marble {

qreal Triangulate::area( const GeoDataLinearRing &polygon )
{

  int n = polygon.size();

  qreal result = 0.0f;

  for ( int p = n - 1, q = 0; q < n; p = q++ ) {
    result += polygon[p].longitude() * polygon[q].latitude() - polygon[q].longitude() * polygon[p].latitude();
  }

  return result * 0.5f;
}

bool Triangulate::isInsideTriangle( const GeoDataCoordinates &A, const GeoDataCoordinates &B,
                                    const GeoDataCoordinates &C, const GeoDataCoordinates &P )
{
  qreal ax = C.longitude() - B.longitude();
  qreal ay = C.latitude() - B.latitude();

  qreal bx = A.longitude() - C.longitude();
  qreal by = A.latitude() - C.latitude();

  qreal cx = B.longitude() - A.longitude();
  qreal cy = B.latitude() - A.latitude();

  qreal apx= P.longitude() - A.longitude();
  qreal apy= P.latitude() - A.latitude();

  qreal bpx= P.longitude() - B.longitude();
  qreal bpy= P.latitude() - B.latitude();

  qreal cpx= P.longitude() - C.longitude();
  qreal cpy= P.latitude() - C.latitude();

  qreal aCROSSbp = ax*bpy - ay*bpx;
  qreal cCROSSap = cx*apy - cy*apx;
  qreal bCROSScp = bx*cpy - by*cpx;

  return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}

bool Triangulate::isEar( const GeoDataLinearRing &polygon, int u, int v, int w, int n, const QVector<int> &V )
{
  const GeoDataCoordinates A = polygon[V[u]];
  const GeoDataCoordinates B = polygon[V[v]];
  const GeoDataCoordinates C = polygon[V[w]];

  // is triangle inside polygon?
  if ( (B.longitude()-A.longitude())*(C.latitude()-A.latitude()) < (B.latitude()-A.latitude())*(C.longitude()-A.longitude()) )
      return false;

  for ( int p = 0; p < n; p++ ) {
    if( (p == u) || (p == v) || (p == w) )
        continue;

    const GeoDataCoordinates P = polygon[V[p]];

    if ( isInsideTriangle( A, B, C, P ) )
        return false;
  }

  return true;
}

QVector<int> Triangulate::Process( const GeoDataLinearRing &polygon )
{
  /* allocate and initialize list of Vertices in polygon */

  const int n = polygon.size();
  QVector<int> V( n );

  /* we want a counter-clockwise polygon in V */
  if ( 0.0f < area( polygon ) )
    for ( int v=0; v<n; v++ )
        V[v] = v;
  else
    for ( int v=0; v<n; v++ )
        V[v] = (n-1)-v;

  QVector<int> result;

  int v = n-1;
  /*  remove nv-2 Vertices, creating 1 triangle every time */
  for ( int count = 2*n, nv = n; nv>2; ) {
    /* if we loop, it is probably a non-simple polygon */
    if ( (count--) <= 0 ) {
      //** Triangulate: ERROR - probable bad polygon!
      return QVector<int>();
    }

    /* three consecutive vertices in current polygon, <u,v,w> */
    v = ( v + 1 ) % nv;
    const int u = ( v + nv - 1 ) % nv;
    const int w = ( v      + 1 ) % nv;

    if ( isEar( polygon, u, v, w, nv, V ) ) {
      /* true names of the vertices */
      const int a = V[u];
      const int b = V[v];
      const int c = V[w];

      /* append Triangle */
      result.append( a );
      result.append( b );
      result.append( c );

      /* remove v from remaining polygon */
      for ( int s = v; s < nv - 1; ++s )
          V[s] = V[s+1];

      nv--;

      /* resest error detection counter */
      count = 2*nv;
    }
  }

  return result;
}

}
