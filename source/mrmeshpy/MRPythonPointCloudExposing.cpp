#include "MRMesh/MRPython.h"
#include "MRMesh/MRPointCloud.h"
#include "MRMesh/MRPointCloudTriangulation.h"
#include "MRMesh/MRMeshToPointCloud.h"
#include "MRMesh/MRBox.h"
#include "MRMesh/MRColor.h"
#include "MRMesh/MRPointsToMeshFusion.h"
#include "MRMesh/MRPointCloudDistance.h"
#include "MRMesh/MRExpected.h"
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace MR
{

MR_ADD_PYTHON_CUSTOM_CLASS( mrmeshpy, PointCloud, PointCloud )

MR_ADD_PYTHON_CUSTOM_DEF( mrmeshpy, PointCloud, [] ( pybind11::module_& m )
{
    MR_PYTHON_CUSTOM_CLASS( PointCloud ).
        def( pybind11::init<>() ).
        def_readwrite( "points", &PointCloud::points ).
        def_readwrite( "normals", &PointCloud::normals ).
        def_readwrite( "validPoints", &PointCloud::validPoints ).
        def( "getBoundingBox", &PointCloud::getBoundingBox, "returns the minimal bounding box containing all valid vertices (implemented via getAABBTree())" ).
        def( "pack", ( bool( PointCloud::* )( VertMap * ) ) &PointCloud::pack, pybind11::arg( "outNew2Old" ) = nullptr,
            "tightly packs all arrays eliminating invalid points, but relative order of valid points is preserved; "
            "returns false if the cloud was packed before the call and nothing has been changed; "
            "if pack is done optionally returns mappings: new.id -> old.id" ).
        def( "invalidateCaches", &PointCloud::invalidateCaches, "Invalidates caches (e.g. aabb-tree) after a change in point cloud" );

    pybind11::class_<TriangulationParameters>( m, "TriangulationParameters", "Parameters of point cloud triangulation" ).
        def( pybind11::init<>() ).
        def_readwrite( "numNeighbours", &TriangulationParameters::numNeighbours,
            "The number of nearest neighbor points to use for building of local triangulation.\n"
            "note: Too small value can make not optimal triangulation and additional holes\n"
            "Too big value increases difficulty of optimization and decreases performance" ).
        def_readwrite( "critAngle", &TriangulationParameters::critAngle, "Critical angle of triangles in local triangulation (angle between triangles in fan should be less then this value)" ).
        def_readwrite( "critHoleLength", &TriangulationParameters::critHoleLength,
            "Critical length of hole (all holes with length less then this value will be filled)\n"
            "If value is subzero it is set automaticly to 0.7*bbox.diagonal()" );

    m.def( "triangulatePointCloud", &triangulatePointCloud,
        pybind11::arg( "pointCloud" ), pybind11::arg_v( "params", TriangulationParameters(), "TriangulationParameters()" ), pybind11::arg( "progressCb" ) = ProgressCallback{},
        "Creates mesh from given point cloud according params\n"
        "Returns empty optional if was interrupted by progress bar" );


    m.def( "meshToPointCloud", &meshToPointCloud,
        pybind11::arg( "mesh" ), pybind11::arg( "saveNormals" ) = true, pybind11::arg( "verts" ) = nullptr,
        "Mesh to PointCloud" );

    pybind11::class_<PointsToMeshParameters>( m, "PointsToMeshParameters", "Parameters of point cloud triangulation" ).
        def( pybind11::init<>() ).
        def_readwrite( "sigma", &PointsToMeshParameters::sigma,
            "The distance of highest influence of a point;\n"
            "the maximal influence distance is 3*sigma; beyond that distance the influence is strictly zero" ).
        def_readwrite( "minWeight", &PointsToMeshParameters::minWeight,
            "minimum sum of influence weights from surrounding points for a triangle to appear,\n"
            "meaning that there shall be at least this number of points in close proximity" ).
        def_readwrite( "voxelSize", &PointsToMeshParameters::voxelSize,
            "Size of voxel in grid conversions;\n"
            "The user is responsible for setting some positive value here" ).
        def_readwrite( "ptColors", &PointsToMeshParameters::ptColors,
            "optional input: colors of input points" ).
        def_readwrite( "vColors", &PointsToMeshParameters::vColors,
            "optional output: averaged colors of mesh vertices" );

    m.def( "pointsToMeshFusion", decorateExpected( &pointsToMeshFusion ),
        pybind11::arg( "pointCloud" ), pybind11::arg_v( "params", PointsToMeshParameters(), "PointsToMeshParameters()" ),
        "Creates mesh from given point cloud according params\n"
        "Returns empty optional if was interrupted by progress bar" );

    m.def("findMaxDistanceSqOneWay",&MR::findMaxDistanceSqOneWay,
        pybind11::arg( "a" ), pybind11::arg( "b" ), pybind11::arg( "rigidB2A" ) = nullptr, pybind11::arg( "upDistLimitSq" ) = FLT_MAX,
        "returns the maximum of the squared distances from each B-point to A-cloud\n"
        "\trigidB2A - rigid transformation from B-cloud space to A-cloud space, nullptr considered as identity transformation\n"
        "\tmaxDistanceSq - upper limit on the positive distance in question, if the real distance is larger than the function exists returning maxDistanceSq" );

    m.def( "findMaxDistanceSq", &MR::findMaxDistanceSq,
        pybind11::arg( "a" ), pybind11::arg( "b" ), pybind11::arg( "rigidB2A" ) = nullptr, pybind11::arg( "upDistLimitSq" ) = FLT_MAX,
        "returns the squared Hausdorff distance between two point clouds, that is the maximum of squared distances from each point to the other cloud (in both directions)\n"
        "\trigidB2A - rigid transformation from B-cloud space to A-cloud space, nullptr considered as identity transformation\n"
        "\tmaxDistanceSq - upper limit on the positive distance in question, if the real distance is larger than the function exists returning maxDistanceSq" );
} )

} //namespace MR

MR_ADD_PYTHON_VEC( mrmeshpy, vectorPointCloud, MR::PointCloud )
