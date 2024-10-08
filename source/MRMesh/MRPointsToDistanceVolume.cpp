#include "MRPointsToDistanceVolume.h"
#include "MRVoxelsVolume.h"
#include "MRVolumeIndexer.h"
#include "MRParallelFor.h"
#include "MRBitSetParallelFor.h"
#include "MRPointCloud.h"
#include "MRPointsInBall.h"
#include "MRColor.h"
#include "MRTimer.h"
#include <limits>

namespace MR
{

Expected<SimpleVolume> pointsToDistanceVolume( const PointCloud & cloud, const PointsToDistanceVolumeParams& params )
{
    MR_TIMER
    assert( params.sigma > 0 );
    assert( params.minWeight > 0 );
    assert( cloud.hasNormals() );

    SimpleVolume res;
    res.voxelSize = params.voxelSize;
    res.dims = params.dimensions;
    VolumeIndexer indexer( res.dims );
    res.data.resize( indexer.size(), std::numeric_limits<float>::quiet_NaN() );

    const auto inv2SgSq = -0.5f / sqr( params.sigma );
    if ( !ParallelFor( size_t( 0 ), indexer.size(), [&]( size_t i )
    {
        auto coord = Vector3f( indexer.toPos( VoxelId( i ) ) ) + Vector3f::diagonal( 0.5f );
        auto voxelCenter = params.origin + mult( params.voxelSize, coord );

        float sumDist = 0;
        float sumWeight = 0;
        findPointsInBall( cloud, voxelCenter, 3 * params.sigma, [&]( VertId v, const Vector3f& p )
        {
            const auto distSq = ( voxelCenter - p ).lengthSq();
            const auto w = std::exp( distSq * inv2SgSq );
            sumWeight += w;
            sumDist += dot( cloud.normals[v], voxelCenter - p ) * w;
        } );

        if ( sumWeight >= params.minWeight )
            res.data[i] = sumDist / sumWeight;
    }, params.cb ) )
        return unexpectedOperationCanceled();

    res.max =  params.sigma * std::exp( -0.5f );
    res.min = -res.max;
    return res;
}

Expected<VertColors> calcAvgColors( const PointCloud & cloud, const VertColors & colors,
    const VertCoords & tgtPoints, const VertBitSet & tgtVerts, float sigma, const ProgressCallback & cb )
{
    MR_TIMER
    assert( sigma > 0 );
    assert( cloud.hasNormals() );

    VertColors res;
    res.resizeNoInit( tgtPoints.size() );

    const auto inv2SgSq = -0.5f / sqr( sigma );
    if ( !BitSetParallelFor( tgtVerts, [&]( VertId tv )
    {
        const auto pos = tgtPoints[tv];

        Vector4f sumColors;
        float sumWeight = 0;
        findPointsInBall( cloud, pos, 3 * sigma, [&]( VertId v, const Vector3f& p )
        {
            const auto distSq = ( pos - p ).lengthSq();
            const auto w = std::exp( distSq * inv2SgSq );
            sumWeight += w;
            sumColors += Vector4f( colors[v] ) * w;
        } );
        if ( sumWeight > 0 )
            res[tv] = Color( sumColors / sumWeight );
    }, cb ) )
        return unexpectedOperationCanceled();

    return res;
}

} //namespace MR
