//
// Created by Morten Nobel-Jørgensen on 22/09/2017.
//

#include "WorldMap.hpp"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <fstream>
#include <iostream>
#include <glm/gtc/matrix_access.inl>

using namespace std;

void WorldMap::loadMap(std::string filename) {
    // todo implement json loading this instead following
/*    values.push_back( { 0, 0, 0,0} ); // -------> x
    values.push_back( { 0,-1,-1,2} ); // |
    values.push_back( { 0,-1,-1,9} ); // |
    values.push_back( { 0, 0, 0,0} ); // v z
    startingPosition.x = 1.5;
    startingPosition.y = 1.5;
    startingRotation = 22.5; */

	using namespace rapidjson;
    ifstream fis(filename);
    IStreamWrapper isw(fis);
    Document d;
    d.ParseStream(isw);

	startingPosition.x = d["spawn"]["x"].GetFloat();
	startingPosition.y = d["spawn"]["y"].GetFloat();
	startingRotation = d["spawn"]["angle"].GetFloat();

	floorColor = glm::vec4(	d["floorColor"].GetArray()[0].GetFloat(),
							d["floorColor"].GetArray()[1].GetFloat(),
							d["floorColor"].GetArray()[2].GetFloat(),
							d["floorColor"].GetArray()[3].GetFloat()
							);

	ceilColor = glm::vec4(d["ceilColor"].GetArray()[0].GetFloat(),
		d["ceilColor"].GetArray()[1].GetFloat(),
		d["ceilColor"].GetArray()[2].GetFloat(),
		d["ceilColor"].GetArray()[3].GetFloat()
		);
	
	
	auto map = d["tileMap"].GetArray();
	for (SizeType y = 0; y <map.Size(); y++){
		auto row = map[y].GetArray();
		vector<int> rowValues;

		for (SizeType x = 0; x <row.Size(); x++) {
			rowValues.push_back(row[x].GetInt());
		}

		values.push_back(rowValues);
	}
}

int WorldMap::getTile(int x, int y) {
    return values.at(y).at(x);
}

int WorldMap::getWidth() {
    return values[0].size();
}

int WorldMap::getHeight() {
    return values.size();
}

glm::vec2 WorldMap::getStartingPosition() {
    return startingPosition;
}

float WorldMap::getStartingRotation() {
    return startingRotation;
}

glm::vec4 WorldMap::getFloorColor() {
    return floorColor;
}

glm::vec4 WorldMap::getCeilColor() {
    return ceilColor;
}