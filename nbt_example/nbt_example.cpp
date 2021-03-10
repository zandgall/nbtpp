// nbt_example.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#define NBT_COMPILE
#include <nbt.hpp>
#include <chrono>
#include <FileChooser.h>
using namespace nbt;
int main() {
	compound tag = compound();
	std::string filepath = ChooseOpenFile("Choose test nbt", "");
	auto start = std::chrono::high_resolution_clock::now();
	auto bytes = readAsBytes(filepath.c_str(), true);
	tag.load((char*)&bytes[0], 0);
	std::string deb = tag.compilation("");
	auto finish = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";
	std::cout << "YES!" << std::endl;
	std::cout << deb << std::endl;
}