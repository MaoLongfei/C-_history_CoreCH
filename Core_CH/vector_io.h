#ifndef VECTOR_IO_H
#define VECTOR_IO_H

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

template<class T>
void save_vector(const std::string&file_name, const std::vector<T>&vec){
	std::ofstream out(file_name, std::ios::binary);
	if(!out)
		throw std::runtime_error("Can not open \""+file_name+"\" for writing.");
	out.write(reinterpret_cast<const char*>(&vec[0]), vec.size()*sizeof(T));
}

template<class T>
std::vector<T>load_vector(const std::string&file_name){
	std::ifstream in(file_name, std::ios::binary);
	if(!in)
		throw std::runtime_error("Can not open \""+file_name+"\" for reading.");
	in.seekg(0, std::ios::end);
	unsigned long long file_size = in.tellg();
	if(file_size % sizeof(T) != 0)
		throw std::runtime_error("File \""+file_name+"\" can not be a vector of the requested type because it's size is no multiple of the element type's size.");
	in.seekg(0, std::ios::beg);
	std::vector<T>vec(file_size / sizeof(T));
	in.read(reinterpret_cast<char*>(&vec[0]), file_size);
	return vec; // NVRO
}

#endif
