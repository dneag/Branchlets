# Branchlets

Classes for creating cylindrical tube meshes in Autodesk Maya, given lists of cylindrical segments defined by a vector and radius.  I use this to create tree branches.

The Branchlets class's most significant job is to calculate the position of every vertex on the mesh, as well as UV positions.  It passes this and the other
needed information to Maya's [MFnMesh::create()](https://help.autodesk.com/view/MAYAUL/2022/ENU/?guid=Maya_SDK_cpp_ref_class_m_fn_mesh_html) method, which is called in the base class, MMesh.

### Usage

Create a list of BSegments and pass them to BranchletCreator::create(...) to instantiate a Branchlets object.  This will create a single branch.  Additional branches can be added with Branchlets::addOne(...).  Each call would take another list of BSegments and create another branch on the mesh.

Note that this repository's master branch has an issue where when segments are facing downwards, vertex rings are sometimes not correctly aligned, resulting in a twisted mesh.  The ResizingSegVectors branch does work in all cases.
