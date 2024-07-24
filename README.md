# Branchlets

A set of classes for using Autodesk Maya's C++ API to make tree branch meshes, given lists of cylindrical segments defined by a vector and radius.

The Branchlets class's most significant job is to calculate the position of every vertex on the mesh, as well as u's and v's.  It then passes this and the other
needed information to Maya's [MFnMesh::create()](https://help.autodesk.com/view/MAYAUL/2022/ENU/?guid=Maya_SDK_cpp_ref_class_m_fn_mesh_html) method, which is called in the base class, MMesh.

Define a list of BSegments, then use BranchletCreator::create(...) to instantiate a Branchlets object.  This will create a single branch.  Additional branches can be added with Branchlets::addOne(...).  Each call would take another list of BSegments, and create another branch on the mesh.

Note that this repository's master branch has an issue where when segments are facing downwards, vertex rings are sometimes not correctly aligned.  The ResizingSegVectors branch does work in all cases.
