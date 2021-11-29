#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

void Mesh::getMtlSize(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;

	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;
		if (var == "newmtl")
			matSize++;
	}
	fin.close();
	printf("MAT %d\n", matSize);
	mat = new Material*[matSize + 1];
	mat[0] = new Material;
}

void Mesh::getSize(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;
	
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;
		if (var == "mtllib") {
			std::string mtlFile;
			fin2 >> mtlFile;
			getMtlSize(mtlFile);
		}
		if (var == "v")
			vSize++;
		if (var == "vt")
			vtSize++;
		if (var == "vn")
			vnSize++;
		if (var == "f") {
			int vertexCnt = 0;
			std::string var;
			while (fin2 >> var)
				vertexCnt++;
			fSize += std::max(0, vertexCnt - 2);
		}
	}
	fin.close();
    printf("get end 1 %d %d \n",vSize, fSize);
	v = new Vector3f[vSize + 1];
	vt = new std::pair<double, double>[vtSize + 1];
	if (vnSize == 0)
		vn = new Vector3f[vSize + 1];
	else
		vn = new Vector3f[vnSize + 1];
	triangleList = new Triangle*[fSize];
}

void Mesh::getMtl(std::string file) {
	std::ifstream fin(file.c_str());
	std::string order;
	int matCnt = 0;
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;

		if (var == "newmtl") {
			std::string matName;
			fin2 >> matName;
			matMap[matName] = ++matCnt;
			mat[matCnt] = new Material();
		}
		if (var == "Ka") {

		}
		if (var == "Kd") {
            Vector3f color(0);
            fin2 >> color[0] >> color[1] >> color[2];
			mat[matCnt]->mColor = color;
			mat[matCnt]->diffidx = mat[matCnt]->mColor.getMAX();
			mat[matCnt]->mColor = mat[matCnt]->mColor/mat[matCnt]->diffidx;
		}
		if (var == "Ks") {
			fin2 >> mat[matCnt]->reflidx;
		}
		if (var == "Tf") {
            Vector3f absorb(0);
            fin2 >> absorb[0] >> absorb[1] >> absorb[2];
			mat[matCnt]->absorb = absorb;
			if (mat[matCnt]->absorb.power() < 1 - EPS) {
				mat[matCnt]->refridx = 1;
			}
		}
		if (var == "Ni") {
			fin2 >> mat[matCnt]->refrN;
		}
	}
	printf("MTL: %d\n", matCnt);
	fin.close();
}

Mesh::Mesh(const char *filename, Material *material, double scale) : Object3D(material) {
	this->scale = scale;
	printf("Scale: [%lf]\n", this->scale);
    printf("Mesh start\n");
    vSize = 0;
	vtSize = 0;
	vnSize = 0;
	fSize = 0;
	matSize = 0;
    tree = new TriangleTree;
    std::string file = std::string(filename);
    getSize(file);
	std::ifstream fin(file.c_str());
	std::string order;

	int matID = -1;
	int vCnt = 0, vtCnt = 0, vnCnt = 0, fCnt = 0;
	while (getline(fin, order, '\n')) {
		std::stringstream fin2(order);
		std::string var;
		if (!(fin2 >> var)) continue;

		if (var == "mtllib") {
			std::string mtlFile;
			fin2 >> mtlFile;
			getMtl(mtlFile);
		}

		if (var == "usemtl") {
			std::string matName;
			fin2 >> matName;
			matID = matMap[matName];
		}
		if (var == "v") {
			vCnt++;
			v[vCnt].Input(fin2);
		}
		if (var == "vt") {
			vtCnt++;
			fin2 >> vt[vtCnt].second >> vt[vtCnt].first;
		}
		if (var == "vn") {
			vnCnt++;
			vn[vnCnt].Input(fin2);
		}
		if (var == "f") {
			Triangle* tri = triangleList[fCnt] = new Triangle(NULL);
			//if (matID != -1)
			//	tri->material = mat[matID];
			//else
				tri->material = this->material;
			std::string str;
			for (int i = 0; fin2 >> str; i++) {
				int bufferLen = 0, buffer[3];
				buffer[0] = buffer[1] = buffer[2] = -1;
				for (int s = 0, t = 0; t < (int)str.length(); t++)
					if (t + 1 >= (int)str.length() || str[t + 1] == '/') {
						buffer[bufferLen++] = atoi(str.substr(s, t - s + 1).c_str());
						s = t + 2;
					}
				int vertexID = i;
				if (i >= 3) {
					vertexID = 2;
					tri = triangleList[fCnt] = new Triangle(NULL);
					*tri = *triangleList[fCnt - 1];
					tri->vertex[1] = tri->vertex[2];
					tri->v2 = tri->v3;
					tri->textureVertex[1] = tri->textureVertex[2];
					tri->normalVectorID[1] = tri->normalVectorID[2];
				}
				if (buffer[0] > 0) {
					tri->vertex[vertexID] = buffer[0];
					Vector3f vertexPos = v[buffer[0]];
                    vertexPos *= 1/this->scale;
                    if (vertexID == 0) tri->v1 = vertexPos;
                    else if (vertexID == 1) tri->v2 = vertexPos;
                    else if (vertexID == 2) tri->v3 = vertexPos;
				}
				if (buffer[1] > 0) {
					tri->textureVertex[vertexID] = buffer[1];
				}
				if (buffer[2] > 0) {
					tri->normalVectorID[vertexID] = buffer[2];
				}
				if (i >= 2) {
					tri->Setparameters();
					fCnt++;
				}
			}
		}
	}
	fin.close();

	TriTreeNode* root = tree->root;
	root->size = fCnt;
	root->triangleList = new Triangle*[root->size];
	for (int i = 0; i < root->size; i++) {
		root->triangleList[i] = triangleList[i];
		root->box.updateBox(triangleList[i]);
	}
	tree->buildTree();
    vertexN = vn;
    pixel = vt;
    printf("vCnt: %d, vtCnt: %d, vnCnt: %d, fCnt: %d, rootSize: %d\n", vCnt, vtCnt, vnCnt, fCnt, root->size);
    printf("Bound Box:\n");
	root->box.maxPos.print();
    root->box.minPos.print();
    printf("Mesh end\n");
}

double BoundBox::intersect(const Ray &r) {
	double minDist = -1;
    Vector3f ray_O = r.getOrigin(), ray_V = r.getDirection();
	for (int idx = 0; idx < 3; idx++) {
		double tmpT = -1;
		if (ray_V[idx] >= EPS)
			tmpT = (minPos[idx] - ray_O[idx]) / ray_V[idx];
		else if (ray_V[idx] <= -EPS)
			tmpT = (maxPos[idx] - ray_O[idx]) / ray_V[idx];
		if (tmpT >= EPS) {
			Vector3f C = ray_O + ray_V * tmpT;
			if (isInBox(C)) {
				double dist = (C - ray_O).length();
				if (minDist <= -EPS || dist < minDist)
					minDist = dist;
			}
		}
	}
	return minDist;
}

void TriangleTree::sortList(Triangle** triangleList, int left, int right, int idx, bool isMin) {
	double (Triangle::*GetCoord)(int) = isMin ? &Triangle::GetMinCoord : &Triangle::GetMaxCoord;
	if (left >= right) return;
	Triangle* key = triangleList[(left + right) >> 1];
	int i,j;
	for (i = left, j = right; i <= j;) {
		while (j >= left && (key->*GetCoord)(idx) < (triangleList[j]->*GetCoord)(idx)) 
			j -= 1;
		while (i <= right && (triangleList[i]->*GetCoord)(idx) < (key->*GetCoord)(idx)) 
			i += 1;
		if (i <= j) {
			std::swap(triangleList[i], triangleList[j]);
			i += 1;
			j -= 1;
		}
	}
	sortList(triangleList, i, right, idx, isMin);
	sortList(triangleList, left, j, idx, isMin);
}

void TriangleTree::putNode(TriTreeNode* node) {
	Triangle** minNode = new Triangle*[node->size];
	Triangle** maxNode = new Triangle*[node->size];
	for (int i = 0; i < node->size; i++) {
		minNode[i] = node->triangleList[i];
		maxNode[i] = node->triangleList[i];
	}
	
	double thisCost = node->box.getArea() * (node->size - 1);
	double minCost = thisCost;
	int bestCoord = -1, leftSize = 0, rightSize = 0;
	double bestSplit = 0;
	for (int idx = 0; idx < 3; idx++) {
		sortList(minNode, 0, node->size - 1, idx, true);
		sortList(maxNode, 0, node->size - 1, idx, false);
		BoundBox leftBox = node->box;
		BoundBox rightBox = node->box;

		int j = 0;
		for (int i = 0; i < node->size; i++) {
			double split = minNode[i]->GetMinCoord(idx);
			leftBox.maxPos[idx] = split;
			rightBox.minPos[idx] = split;
			for ( ; j < node->size && maxNode[j]->GetMaxCoord(idx) <= split + EPS; j++);
			double cost = leftBox.getArea() * i + rightBox.getArea() * (node->size - j);
			if (cost < minCost) {
				minCost = cost;
				bestCoord = idx;
				bestSplit = split;
				leftSize = i;
				rightSize = node->size - j;
			}
		}

		j = 0;
		for (int i = 0; i < node->size; i++) {
			double split = maxNode[i]->GetMaxCoord(idx);
			leftBox.maxPos[idx] = split;
			rightBox.minPos[idx] = split;
			for ( ; j < node->size && minNode[j]->GetMinCoord(idx) <= split - EPS; j++);
			double cost = leftBox.getArea() * j + rightBox.getArea() * (node->size - i);
			if (cost < minCost) {
				minCost = cost;
				bestCoord = idx;
				bestSplit = split;
				leftSize = j;
				rightSize = node->size - i;
			}
		}
	}

	delete minNode;
	delete maxNode;

	if (bestCoord != -1) {
		leftSize = rightSize = 0;
		for (int i = 0; i < node->size; i++) {
			if (node->triangleList[i]->GetMinCoord(bestCoord) <= bestSplit - EPS || node->triangleList[i]->GetMaxCoord(bestCoord) <= bestSplit + EPS)
				leftSize++;
			if (node->triangleList[i]->GetMaxCoord(bestCoord) >= bestSplit + EPS || node->triangleList[i]->GetMinCoord(bestCoord) >= bestSplit - EPS)
				rightSize++;
		}
		BoundBox leftBox = node->box;
		BoundBox rightBox = node->box;
		leftBox.maxPos[bestCoord] = bestSplit;
		rightBox.minPos[bestCoord] = bestSplit;
		double cost = leftBox.getArea() * leftSize + rightBox.getArea() * rightSize;

		if (cost < thisCost) {
			node->plane = bestCoord;
			node->split = bestSplit;

			node->leftNode = new TriTreeNode;
			node->leftNode->box = node->box;
			node->leftNode->box.maxPos[node->plane] = node->split;
			
			node->rightNode = new TriTreeNode;
			node->rightNode->box = node->box;
			node->rightNode->box.minPos[node->plane] = node->split;
			
			node->leftNode->triangleList = new Triangle*[leftSize];
			node->rightNode->triangleList = new Triangle*[rightSize];
			int leftCnt = 0, rightCnt = 0;
			for (int i = 0; i < node->size; i++) {
                
				if (node->triangleList[i]->GetMinCoord(node->plane) <= node->split - EPS || node->triangleList[i]->GetMaxCoord(node->plane) <= node->split + EPS)
					node->leftNode->triangleList[leftCnt++] = node->triangleList[i];
				if (node->triangleList[i]->GetMaxCoord(node->plane) >= node->split + EPS || node->triangleList[i]->GetMinCoord(node->plane) >= node->split - EPS)
					node->rightNode->triangleList[rightCnt++] = node->triangleList[i];
			}
			node->leftNode->size = leftSize;
			node->rightNode->size = rightSize;

			putNode(node->leftNode);
			putNode(node->rightNode);
		}
	}
}

bool TriangleTree::searchTree(TriTreeNode* node, const Ray &r, Hit &h, double tmin){
    Vector3f ray_O = r.getOrigin(), ray_V = r.getDirection();
	if (!node->box.isInBox(ray_O) && node->box.intersect(r) <= -EPS)
		return false;
    bool treeInterscet = false;
    
	if (node->leftNode == NULL && node->rightNode == NULL) {
		for (int i = 0; i < node->size; i++) {
            bool triIntersect;
            Hit triHit = Hit();
			triIntersect = node->triangleList[i]->intersect(r, triHit, tmin);
			if (triIntersect && node->box.isInBox(r.pointAtParameter(triHit.getT())) && (!treeInterscet || triHit.getT() < h.getT())){
				h = triHit;
                treeInterscet = true;
            }
		}
		return treeInterscet;
	}
	
	if (node->leftNode->box.isInBox(ray_O)) {
		treeInterscet = searchTree(node->leftNode, r, h, tmin);
		if (treeInterscet) return treeInterscet;
		return searchTree(node->rightNode, r, h, tmin);
	}
	if (node->rightNode->box.isInBox(ray_O)) {
        treeInterscet = searchTree(node->rightNode, r, h, tmin);
		if (treeInterscet) return treeInterscet;
		return searchTree(node->leftNode, r, h, tmin);
	}

	double leftDist = node->leftNode->box.intersect(r);
	double rightDist = node->rightNode->box.intersect(r);
	if (rightDist <= -EPS)
		return searchTree(node->leftNode, r, h, tmin);
	if (leftDist <= -EPS)
		return searchTree(node->rightNode, r, h, tmin);
	
	if (leftDist < rightDist) {
        treeInterscet = searchTree(node->leftNode, r, h, tmin);
		if (treeInterscet) return treeInterscet;
		return searchTree(node->rightNode, r, h, tmin);
	}
	treeInterscet = searchTree(node->rightNode, r, h, tmin);
    if (treeInterscet) return treeInterscet;
    return searchTree(node->leftNode, r, h, tmin);
}

bool Mesh::intersect(const Ray &r, Hit &h, double tmin) {
    bool isHit =  tree->intersect(r, h, tmin);
    return isHit;  
}