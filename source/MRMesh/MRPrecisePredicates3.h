#pragma once

#include "MRVector3.h"
#include "MRId.h"

#include <array>

namespace MR
{

/// \ingroup MathGroup
/// \{

/// returns true if the plane with orientated triangle ABC has 0 point at the left;
/// uses simulation-of-simplicity to avoid "0 is exactly on plane"
MRMESH_API bool orient3d( const Vector3i & a, const Vector3i & b, const Vector3i & c );

/// returns true if the plane with orientated triangle ABC has D point at the left;
/// uses simulation-of-simplicity to avoid "D is exactly on plane"
inline bool orient3d( const Vector3i & a, const Vector3i & b, const Vector3i & c, const Vector3i & d )
    { return orient3d( a - d, b - d, c - d ); }

struct PreciseVertCoords
{
    VertId id;   ///< unique id of the vertex (in both meshes)
    Vector3i pt; ///< integer coordinates of the vertex
};

/// first sorts the indices in ascending order, then calls the predicate for sorted points
MRMESH_API bool orient3d( const std::array<PreciseVertCoords, 4> & vs );
MRMESH_API bool orient3d( const PreciseVertCoords* vs );

struct TriangleSegmentIntersectResult
{
    bool doIntersect = false;    ///< whether triangle and segment intersect
    bool dIsLeftFromABC = false; ///< whether the plane with orientated triangle ABC has D point at the left

    explicit operator bool() const { return doIntersect; }
};

/// checks whether triangle ABC (indices 012) and segment DE (indices 34) intersect
/// uses simulation-of-simplicity to avoid edge-segment intersections and co-planarity
[[nodiscard]] MRMESH_API TriangleSegmentIntersectResult doTriangleSegmentIntersect(
    const std::array<PreciseVertCoords, 5> & vs );

/// float-to-int coordinate converter
using ConvertToIntVector = std::function<Vector3i( const Vector3f& )>;
/// int-to-float coordinate converter
using ConvertToFloatVector = std::function<Vector3f( const Vector3i& )>;
/// this struct contains coordinate converters float-int-float
struct CoordinateConverters
{
    ConvertToIntVector toInt{};
    ConvertToFloatVector toFloat{};
};

/// creates converter from Vector3f to Vector3i in Box range (int diapason is mapped to box range)
MRMESH_API ConvertToIntVector getToIntConverter( const Box3d& box );
/// creates converter from Vector3i to Vector3f in Box range (int diapason is mapped to box range)
MRMESH_API ConvertToFloatVector getToFloatConverter( const Box3d& box );

/// finds intersection precise, using high precision int inside
/// this function input should have intersection
[[nodiscard]] MRMESH_API Vector3f findTriangleSegmentIntersectionPrecise( 
    const Vector3f& a, const Vector3f& b, const Vector3f& c,
    const Vector3f& d, const Vector3f& e, 
    CoordinateConverters converters );

/// \}

}
