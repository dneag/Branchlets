
#include "MMesh.h"

MStatus MMesh::createMesh(std::string name) const {

	MStatus status = MS::kSuccess;

	if (verts.length() > 2) {

		MFnMesh newMesh;
		MObject transform = newMesh.create(verts.length(), faceCounts.length(), verts, faceCounts, faceConnects, us, vs);
		newMesh.assignUVs(faceCounts, uvConnects);

		MFnDependencyNode nodeFn;
		nodeFn.setObject(transform);
		nodeFn.setName(name.c_str());
	}
	else {

		MGlobal::displayInfo(MString() + "No mesh created for " + name.c_str() + " because it has " + verts.length() + " vertices");
	}


	return status;
}