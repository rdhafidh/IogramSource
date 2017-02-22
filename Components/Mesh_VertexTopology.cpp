#include "Mesh_VertexTopology.h"
#include "IoDataTree.h"

#include "TriMesh.h"
#include "MeshTopologyQueries.h"


using namespace Urho3D;

String Mesh_VertexTopology::iconTexture = "Textures/Icons/Mesh_VertexTopology.png";

Mesh_VertexTopology::Mesh_VertexTopology(Context* context) : IoComponentBase(context, 1, 3)
{
    SetName("VertexTopology");
    SetFullName("Compute Mesh Vertex Topology");
    SetDescription("Computes Mesh Topology Data for vertices");
    SetGroup(IoComponentGroup::MESH);
    SetSubgroup("Analysis");
    
    inputSlots_[0]->SetName("Mesh");
    inputSlots_[0]->SetVariableName("M");
    inputSlots_[0]->SetDescription("TriMeshWithAdjacencyData");
    inputSlots_[0]->SetVariantType(VariantType::VAR_VARIANTMAP);
    inputSlots_[0]->SetDataAccess(DataAccess::ITEM);
    
    outputSlots_[0]->SetName("VertexStarIDs");
    outputSlots_[0]->SetVariableName("V_ID");
    outputSlots_[0]->SetDescription("Indices of vertices adjacent to V");
    outputSlots_[0]->SetVariantType(VariantType::VAR_INT);
    outputSlots_[0]->SetDataAccess(DataAccess::LIST);
    
    outputSlots_[1]->SetName("VertexStar");
    outputSlots_[1]->SetVariableName("V");
    outputSlots_[1]->SetDescription("Vertices adjacent to V");
    outputSlots_[1]->SetVariantType(VariantType::VAR_VECTOR3);
    outputSlots_[1]->SetDataAccess(DataAccess::LIST);
    
    outputSlots_[2]->SetName("AdjFaces");
    outputSlots_[2]->SetVariableName("F");
    outputSlots_[2]->SetDescription("Indices of faces incident to V");
    outputSlots_[2]->SetVariantType(VariantType::VAR_INT);
    outputSlots_[2]->SetDataAccess(DataAccess::LIST);
    
}

int Mesh_VertexTopology::LocalSolve()
{
    assert(inputSlots_.Size() >= 1);
    assert(outputSlots_.Size() == 3);
    
    if (inputSlots_[0]->HasNoData()) {
        solvedFlag_ = 0;
        return 0;
    }
    
    IoDataTree inputMeshTree = inputSlots_[0]->GetIoDataTree();

    Variant inMesh;
    Urho3D::Vector<int> path;
    path.Push(0);
    
    // for some reason the component was crashing on initialization without this:
    inputMeshTree.GetItem(inMesh, path, 0);
    if (inMesh.GetType() == VAR_NONE)
    {
        solvedFlag_ = 0;
        return 0;
    }
    
    IoDataTree vertex_star_vectors(GetContext());
    IoDataTree vertex_stars = ComputeVertexStars(inMesh, vertex_star_vectors);
    IoDataTree adjacent_faces = ComputeAdjacentFaces(inMesh);
    
    outputSlots_[0]->SetIoDataTree(vertex_stars);
    outputSlots_[1]->SetIoDataTree(vertex_star_vectors);
    outputSlots_[2]->SetIoDataTree(adjacent_faces);
    
    solvedFlag_ = 1;
    return 1;
}

IoDataTree Mesh_VertexTopology::ComputeVertexStars(Urho3D::Variant inMeshWithData, IoDataTree& starVectorsTree)
{
    IoDataTree vertex_stars_tree(GetContext());
    
    VariantMap* meshWithData = inMeshWithData.GetVariantMapPtr();
    VariantMap* triMesh = (*meshWithData)["mesh"].GetVariantMapPtr();
    
    Urho3D::VariantVector vertexList = TriMesh_GetVertexList(Variant(*triMesh));
    
    for (int i = 0; i < vertexList.Size(); ++i){
        Vector<int> path;
        path.Push(i);
        
        VariantVector* vertex_vertex_list = (*meshWithData)["vertex-vertex"].GetVariantVectorPtr();
        int num = vertex_vertex_list->Size();
        if (i < num ){
            VariantVector vertex_star = (*vertex_vertex_list)[i].GetVariantVector();
            vertex_stars_tree.Add(path, vertex_star);
            
            // compute the vectors of these verts at the same time.
            VariantVector star_vectors;
            for (int j = 0; j < vertex_star.Size(); ++j){
                int curVert = vertex_star[j].GetInt();
                star_vectors.Push(Variant(vertexList[curVert].GetVector3()));
            }
            starVectorsTree.Add(path, star_vectors);
        }
    }

    return vertex_stars_tree;
}

IoDataTree Mesh_VertexTopology::ComputeAdjacentFaces(Urho3D::Variant inMeshWithData)
{
    IoDataTree adjacent_faces_tree(GetContext());
    
    VariantMap* meshWithData = inMeshWithData.GetVariantMapPtr();
    VariantMap* triMesh = (*meshWithData)["mesh"].GetVariantMapPtr();
    
    Urho3D::VariantVector vertexList = TriMesh_GetVertexList(Variant(*triMesh));
    
    for (int i = 0; i < vertexList.Size(); ++i){
        Vector<int> path;
        path.Push(i);
        
        VariantVector* vertex_face_list = (*meshWithData)["vertex-face"].GetVariantVectorPtr();
        int num = vertex_face_list->Size();
        if (i < num){
            VariantVector adj_faces = (*vertex_face_list)[i].GetVariantVector();
            adjacent_faces_tree.Add(path, adj_faces);
        }
        
    }
    
    return adjacent_faces_tree;
}
