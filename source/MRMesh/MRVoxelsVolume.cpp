#include "MRVoxelsVolume.h"
#include "MRExpected.h"
#include "MRTimer.h"
#include "MRVolumeIndexer.h"
#include "MRParallelFor.h"



namespace MR
{

Expected<SimpleVolume> functionVolumeToSimpleVolume( const FunctionVolume& volume, const ProgressCallback& cb )
{
    MR_TIMER
    SimpleVolume res;
    res.voxelSize = volume.voxelSize;
    res.dims = volume.dims;
    VolumeIndexer indexer( res.dims );
    res.data.resize( indexer.size() );

    if ( !ParallelFor( size_t( 0 ), indexer.size(), [&]( size_t i )
    {
        res.data[i] = volume.data( indexer.toPos( VoxelId( i ) ) );
    }, cb ) )
        return unexpectedOperationCanceled();

    std::tie( res.min, res.max ) = parallelMinMax( res.data );
    return res;
}

}