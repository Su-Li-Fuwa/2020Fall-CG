#ifndef MESH_H
#define MESH_H
#include <vector>
#include <map>
#include <string>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"


class BoundBox {
public:
	Vector3f minPos, maxPos;
	BoundBox(){
		minPos = Vector3f(INFINITY, INFINITY, INFINITY);
		maxPos = Vector3f(-INFINITY, -INFINITY, -INFINITY);
	}
	void updateBox(Triangle* tri){
		for (int idx = 0; idx < 3; idx++) {
			if (tri->GetMinCoord(idx) < minPos[idx]) minPos[idx] = tri->GetMinCoord(idx);
			if (tri->GetMaxCoord(idx) > maxPos[idx]) maxPos[idx] = tri->GetMaxCoord(idx);
		}
	}
	bool isInBox(Vector3f ori){
		for (int idx = 0; idx < 3; idx++)
			if (ori[idx] <= minPos[idx] - EPS || ori[idx] >= maxPos[idx] + EPS) return false;
		return true;
	}
	double getArea(){
		double a = maxPos[0] - minPos[0];
		double b = maxPos[1] - minPos[1];
		double c = maxPos[2] - minPos[2];
		return 2 * (a * b + b * c + c * a);
	}
    double intersect(const Ray &r);
};

class TriTreeNode {
public:
	Triangle** triangleList;
	int size, plane;
	double split;
	BoundBox box;
	TriTreeNode* leftNode;
	TriTreeNode* rightNode;

	TriTreeNode(){
		size = 0;
		plane = -1;
		split = 0;
		leftNode = rightNode = NULL;
	}
	~TriTreeNode(){
		for (int i = 0; i < size; i++)
			delete triangleList[i];
		delete triangleList;
		delete leftNode;
		delete rightNode;
	}
};


class TriangleTree {
	void deleteTree(TriTreeNode* node){
		if (node->leftNode != NULL)
			deleteTree(node->leftNode);
		if (node->rightNode != NULL)
			deleteTree(node->rightNode);
		delete node;
	}
	void sortList(Triangle** triangleList, int l, int r, int idx, bool minCoord);
	void putNode(TriTreeNode* node);
	bool searchTree(TriTreeNode* node, const Ray &r, Hit &h, double tmin);

public:
	TriTreeNode* root;
	TriangleTree(){
		root = new TriTreeNode;
	}
	~TriangleTree(){
		deleteTree(root);
	}

	void buildTree(){
		putNode(root);
	}
	bool intersect(const Ray &r, Hit &h, double tmin){
		return searchTree(root, r, h, tmin);
	}
};


class Mesh : public Object3D {
public:
    Mesh(const char *filename, Material *m, double scale);

	TriangleTree* tree;
	Vector3f* vertexN;
	std::pair<double, double>* pixel;
	double scale=0.3;
    bool intersect(const Ray &r, Hit &h, double tmin) override;
	void getSize(std::string file);
	void getMtlSize(std::string file);
	void getMtl(std::string file);

private:
	int vSize, vtSize, vnSize, fSize, matSize;
	Vector3f* v;
	std::pair<double, double>* vt;
	Vector3f* vn;
	Triangle** triangleList;
	Material** mat;
	std::map<std::string, int> matMap;	

};
#endif