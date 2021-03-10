#pragma once
#ifndef NBT
#define NBT
#include <vector>
#include <exception>
#include <typeinfo>
#include <string>
//#include <chrono>
#include <iostream>
namespace nbt {
	class invalid_tag_id_exception : public std::exception {
	public:
		char id, required_type;
		invalid_tag_id_exception(char id, char required_type) {
			this->id = id;
			this->required_type = required_type;
		}
		const char* what() {
			return ("Invalid tag ID exception. Tag ID read (" + std::to_string((int)id) + ") isn't the same as the Tag that's reading it! (" + std::to_string((int)required_type) + ")").c_str();
		}
	};
	class illegal_list_tag_type : public std::exception {
	public:
		char type;
		illegal_list_tag_type(char type) {
			this->type = type;
		}
		const char* what() {
			return ("List tag cannot contain tags of this type! (" + std::to_string((int)type) + ")").c_str();
		}
	};
	class missing_tag_id_exception : public std::exception {
	public:
		char id;
		missing_tag_id_exception(char id) {
			this->id = id;
		}
		const char* what() {
			return ("Tried to load tag, but didn't find a valid tag, intead got " + std::to_string((int)id)).c_str();
		}
	};
	class tag {
	public:
		// The NBT id of the 
		char id = -1;
		// The name of the tag
		const char* name = "";
		/// <summary>
		/// Load data to this tag from the list of bytes, starting at the given offset. Used by nbt::compound tags to load sub-tags
		/// </summary>
		/// <param name="bytes">List of NBT data. Equivelent to a decompressed .dat file.</param>
		/// <param name="offset">The position to start reading data from</param>
		/// <returns>Where the current tag's data ends, and the next tag's data begins.</returns>
		virtual size_t load(char* bytes, size_t offset = 0) = 0;
		/// <summary>
		/// Writes tag's data to an output buffer. Buffer is able to then be saved to a file or loaded by other tags.
		/// </summary>
		/// <param name="buffer">Where the tag's data will be written to, make sure it has appropriate space. If given nullptr, this function can be used to get the exact length required for a buffer (FASTER+SAFER TO USE WRITE WITH A LIST INSTEAD OF CHAR ARRAY)</param>
		/// <param name="offset">Where the tag should start writing data</param>
		/// <returns>Where the current tag's data ends, and the next tag's data should begin</returns>
		virtual size_t write(char* buffer, size_t offset) = 0;
		/// <summary>
		/// Writes tag's data to an extendable output buffer. Buffer is able to then be saved to a file or loaded by other tags.
		/// </summary>
		/// <param name="buffer">Where the tag's data will be written to, pushing back the end of the buffer.</param>
		/// <returns>Where the current tag's data ends, and the next tag's data should begin</returns>
		virtual size_t write(std::vector<char>& buffer) = 0;
		virtual std::vector<char> value_bytes() = 0;
#ifdef NBT_COMPILE
		virtual std::string compilation(std::string regex = "") = 0;
#endif

		//operator tag* () {
			//return this;
		//}
	};
	class end : public tag {
	public:
		end() {
			id = 0;
			name = "";
		}

		size_t load(char* bytes, size_t offset) {
			return offset + 1;
		}
		size_t write(std::vector<char>& buffer) {
			buffer.push_back(0);
			return buffer.size();
		}
		size_t write(char* buffer, size_t offset) {
			buffer[offset] = 0;
			return offset + 1;
		}
		std::vector<char> value_bytes() {
			return { 0 };
		}
#ifdef NBT_COMPILE
		std::string compilation(std::string regex) {
			return "END\n";
		};
#endif
	};

	// Abstract class used for primive tags such as: IntTag, FloatTag, ByteTag, DoubleTag, etc.
	template <typename T>
	class primitivetag : public tag {
	public:
		T data;
		primitivetag() {
			data = T();
			if (typeid(T) == typeid(char))
				id = 1;
			else if (typeid(T) == typeid(short))
				id = 2;
			else if (typeid(T) == typeid(int))
				id = 3;
			else if (typeid(T) == typeid(long long) || typeid(T) == typeid(long))
				id = 4;
			else if (typeid(T) == typeid(float))
				id = 5;
			else if (typeid(T) == typeid(double))
				id = 6;
			else id = 255;
		}
		primitivetag(const char* name) : primitivetag() {
			this->name = name;
		}
		primitivetag(T data) : primitivetag() {
			this->data = data;
		}
		primitivetag(T data, const char* name) : primitivetag() {
			this->name = name;
			this->data = data;
		}
		primitivetag(const char* name, T data) : primitivetag() {
			this->name = name;
			this->data = data;
		}
		primitivetag(tag* tag) {
			primitivetag<T>* t = dynamic_cast<primitivetag<T>*>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}
		size_t load(char* bytes, size_t offset) {

			// Grabs the id of the tag
			id = bytes[offset];

			// Checks id against what the id of this tag type should be, and throw an exception if it's invalid
			if (id != (char)255 && this->id != (char)255 && id != this->id) {
				throw invalid_tag_id_exception(id, this->id);
				return 0;
			}

			// Get the length of the name
			unsigned short namelength = ((unsigned short)bytes[offset + 1] << 8) + (unsigned char)bytes[offset + 2];

			// Copy the name out of the bytes nbt data array
			name = new const char[namelength + 1];
			strncpy((char*)name, (const char*)&bytes[offset + 3], namelength);
			((char*)name)[namelength] = '\0';

			// Declare what will be the bytes of the tag's data
			char* dat = new char[sizeof(T)];
			switch (sizeof(T)) {
			case 1:
				dat[0] = bytes[offset + 3 + namelength];
				break;
			case 2:
				dat[0] = bytes[offset + 4 + namelength];
				dat[1] = bytes[offset + 3 + namelength];
				break;
			case 4:
				dat[0] = bytes[offset + 6 + namelength];
				dat[1] = bytes[offset + 5 + namelength];
				dat[2] = bytes[offset + 4 + namelength];
				dat[3] = bytes[offset + 3 + namelength];
				break;
			case 8:
				dat[0] = bytes[offset + 10 + namelength];
				dat[1] = bytes[offset + 9 + namelength];
				dat[2] = bytes[offset + 8 + namelength];
				dat[3] = bytes[offset + 7 + namelength];
				dat[4] = bytes[offset + 6 + namelength];
				dat[5] = bytes[offset + 5 + namelength];
				dat[6] = bytes[offset + 4 + namelength];
				dat[7] = bytes[offset + 3 + namelength];
				break;
			default:
				// Loop through the size of primitive type T, and set the equivelent byte of dat to the bytes of the nbt data
				for (char i = 0; i < sizeof(T); i++)
					dat[i] = bytes[offset + 2 + sizeof(T) - i + namelength];
			}

			// Use primitive casting to convert the data (in bytes) to the data (as a primitive)
			// This allows us to use floating point types with primitive tag. The other way to convert to primitive (by using byte shifting "<<") will only convert to integer types
			data = *(T*)(dat);

			// Clear space
			delete[] dat;

			//Return where the next tag will start in bytes[]
			return offset + 3 + sizeof(T) + namelength;

		}
		size_t write(char* buffer, size_t offset) {
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			strncpy(&buffer[offset + 3], name, namelength);

			// Convert the data into byte form, and copy it to the buffer
			union t {
				char* b;
				T ty;
			} o;
			o.ty = data;
			char* dat = o.b;
			strncpy(&buffer[offset + 3 + namelength], (const char*)dat, sizeof(T));

			//Return where the next tag will start in buffer[]
			return offset + 3 + sizeof(T) + namelength;
		}
		size_t write(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			for (char i = 0; i < namelength; i++)
				buffer.push_back(name[i]);

			// Convert the data into byte form, and copy it to the buffer
			union t {
				char* b[sizeof(T)];
				T ty;
			} o;
			o.ty = data;
			//strncpy(&buffer[offset + 3 + namelength], dat, sizeof(T));
			for (char i = 0; i < sizeof(T); i++)
				buffer.push_back((char)o.b[i]);

			//Return where the next tag will start in buffer[]
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			union t {
				char* b[sizeof(T)];
				T ty;
			} o;
			o.ty = data;
			std::vector<char> out = std::vector<char>();
			for (char i = 0; i < sizeof(T); i++)
				out.push_back((char)o.b[i]);
			return out;
		}

		// CONVERSION AND SYNTAX SIMPLIFICATION
		operator T() {
			return data;
		}
		operator T* () {
			return &data;
		}
		operator primitivetag<T>* () {
			return this;
		}

#ifdef NBT_COMPILE
		std::string compilation(std::string regex) {
			std::string type = "";
			if (typeid(T) == typeid(char))
				type = "Byte";
			else if (typeid(T) == typeid(short))
				type = "Short";
			else if (typeid(T) == typeid(int))
				type = "Int";
			else if (typeid(T) == typeid(long long) || typeid(T) == typeid(long))
				type = "Long";
			else if (typeid(T) == typeid(float))
				type = "Float";
			else if (typeid(T) == typeid(double))
				type = "Double";
			else
				type = "Custom";
			std::string out = regex + type + "Tag(" + std::string(name) + "): " + std::to_string(data) + "\n";
			return out;
		}
#endif

	private:
		// Compares the current tag-id to what the tag is
		bool correct_tag() {
			if (typeid(T) == typeid(char))
				return id == 1;
			else if (typeid(T) == typeid(short))
				return id == 2;
			else if (typeid(T) == typeid(int))
				return id == 3;
			else if (typeid(T) == typeid(long long) || typeid(T) == typeid(long))
				return id == 4;
			else if (typeid(T) == typeid(float))
				return id == 5;
			else if (typeid(T) == typeid(double))
				return id == 6;
			else
				return true; // Return true to allow for custom primitive tags
		}
	};

	template <typename T>
	class primitivearraytag : public tag {
	public:
		std::vector<T> data;
		primitivearraytag() {
			this->data = std::vector<T>();
			if (typeid(T) == typeid(char))
				id = 7;
			else if (typeid(T) == typeid(int))
				id = 3;
			else if (typeid(T) == typeid(long long) || typeid(T) == typeid(long))
				id = 4;
			else id = 255;
		}
		primitivearraytag(std::vector<T> data) : primitivearraytag() {
			this->data = data;
		}
		primitivearraytag(const char* name) : primitivearraytag() {
			this->data = std::vector<T>();
			this->name = name;
		}
		primitivearraytag(const char* name, std::vector<T> data) : primitivearraytag() {
			this->data = data;
			this->name = name;
		}
		primitivearraytag(std::vector<T> data, const char* name) : primitivearraytag() {
			this->data = data;
			this->name = name;
		}
		primitivearraytag(tag* tag) : primitivearraytag() {
			primitivearraytag<T>* t = dynamic_cast<primitivearraytag<T>*>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}
		size_t load(char* bytes, size_t offset) {
			// Grabs the id of the tag
			id = bytes[offset];

			// Checks id against what the id of this tag type should be, and throw an exception if it's invalid
			if (id != '\255' && this->id != '\255' && id != this->id) {
				throw invalid_tag_id_exception(id, this->id);
				return 0;
			}

			// Get the length of the name
			unsigned short namelength = ((unsigned short)bytes[offset + 1] << 8) + (unsigned char)bytes[offset + 2];

			// Copy the name out of the bytes nbt data array
			name = new const char[namelength + 1];
			strncpy((char*)name, (const char*)&bytes[offset + 3], namelength);
			((char*)name)[namelength] = '\0';

			// Get the amount of elements in the array
			unsigned int length = ((unsigned int)bytes[offset + 3 + namelength] << 24) + ((unsigned int)bytes[offset + 4 + namelength] << 16) + ((unsigned short)bytes[offset + 5 + namelength] << 8) + (unsigned char)bytes[offset + 6 + namelength];
			// Initiate the array as empty
			data = std::vector<T>();
			// Edit offset for simplicity
			offset += 7 + namelength;
			// Loop through each element and push them into data
			char* dat = new char[sizeof(T)];
			for (int i = 0; i < length; i++) {
				// Create an array to store the current element into, as an array of bytes
				switch (sizeof(T)) {
				case 1:
					dat[0] = bytes[offset + 0];
					break;
				case 2:
					dat[0] = bytes[offset + 1];
					dat[1] = bytes[offset + 0];
					break;
				case 4:
					dat[0] = bytes[offset + 3];
					dat[1] = bytes[offset + 2];
					dat[2] = bytes[offset + 1];
					dat[3] = bytes[offset + 0];
					break;
				case 8:
					dat[0] = bytes[offset + 7];
					dat[1] = bytes[offset + 6];
					dat[2] = bytes[offset + 5];
					dat[3] = bytes[offset + 4];
					dat[4] = bytes[offset + 3];
					dat[5] = bytes[offset + 2];
					dat[6] = bytes[offset + 1];
					dat[7] = bytes[offset + 0];
					break;
				default:
					// Loop through the current element's size and put it's byte value into the dat array
					for (char j = 0; j < sizeof(T); j++)
						dat[sizeof(T) - 1 - j] = bytes[offset + j];
				}
				// Add the current data into the list, and change the offset for the next loop
				data.push_back(*(T*)(dat));
				offset += sizeof(T);
			}
			// Delete old dat array
			delete[] dat;
			return offset;
		}
		size_t write(char* buffer, size_t offset) {
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			strncpy(&buffer[offset + 3], name, namelength);

			// Convert the length into byte form, and copy it to the buffer
			union t {
				char* b;
				T ty;
			} o;
			o.ty = data.size();
			char* len = o.b;
			strncpy(&buffer[offset + 3 + namelength], (const char*)len, sizeof(size_t));

			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				// Use same method to convert the primitive into a char array
				union t {
					char* b;
					T ty;
				} o;
				o.ty = data[i];
				char* dat = o.b;
				strncpy(&buffer[offset + 7 + namelength], (const char*)dat, sizeof(T));
				// Change offset for future use
				offset += sizeof(T);
			}

			//Return where the next tag will start in buffer[]
			return offset;
		}
		size_t write(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			for (char i = 0; i < namelength; i++)
				buffer.push_back(name[i]);

			// Convert the data into byte form, and copy it to the buffer
			char* len = reinterpret_cast<char*>(data.size());
			//strncpy(&buffer[offset + 3 + namelength], dat, sizeof(T));
			for (char i = 0; i < sizeof(size_t); i++)
				buffer.push_back(len[i]);

			// Loop through all elements of data and copy them into the buffer
			for (int i = 0; i < data.size(); i++) {
				// Use same method to convert the primitive into a char array
				char* dat = reinterpret_cast<char*>(data[i]);
				for (char i = 0; i < sizeof(T); i++)
					buffer.push_back(dat[i]);
			}

			//Return where the next tag will start in buffer[]
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> out = std::vector<char>();
			//union t {
			//	char* b[sizeof(T)];
			//	T ty;
			//};
			//for (int i = 0; i < data.size(); i++) {
			//	t o;
			//	o.ty = data[i];
			//	for (char i = 0; i < sizeof(T); i++)
			//		out.push_back((char)o.b[i]);
			//}
			for (int i = 0; i < data.size(); i++) {
				char* list = reinterpret_cast<char*>(&data[i]);
				for (char o = 0; o < sizeof(T); o++)
					out.push_back(list[o]);
			}
			return out;
		}

		// CONVERSIONS AND SYNTAX SIMPLIFICATION
		void operator<<(T t) {
			data.push_back(t);
		}
		T operator[](size_t i) {
			return data[i];
		}
		operator std::vector<T>() {
			return data;
		}
		//operator T* () {
			//return &data[0];
		//}
#ifdef NBT_COMPILE
		std::string compilation(std::string regex) {
			std::string type = "";
			if (typeid(T) == typeid(char))
				type = "Byte";
			else if (typeid(T) == typeid(int))
				type = "Int";
			else if (typeid(T) == typeid(long long) || typeid(T) == typeid(long))
				type = "Long";
			else
				type = "Custom";
			std::string out = regex + type + "Tag(" + std::string(name) + "): " + std::to_string(data.size()) + " " + type + "s \n";
			return out;
		}
#endif

	private:
		// Compares the current tag-id to what the tag is
		char correct_tag() {
			if (typeid(T) == typeid(char))
				return 7;
			else if (typeid(T) == typeid(int))
				return 11;
			else if (typeid(T) == typeid(long long) || typeid(T) == typeid(long))
				return 12;
			else
				return 255; // Return true to allow for custom primitive tags
		}
	};

	typedef primitivetag<char> bytetag;				//1
	typedef primitivetag<short> shorttag;			//2
	typedef primitivetag<int> inttag;				//3
	typedef primitivetag<long long> longtag;		//4
	typedef primitivetag<float> floattag;			//5
	typedef primitivetag<double> doubletag;			//6
	typedef primitivearraytag<char> bytearray;
	typedef primitivearraytag<int> intarray;		//11
	typedef primitivearraytag<long long> longarray;	//12

	class stringtag : public tag {
	public:
		std::string data;
		stringtag() {
			data = std::string();
			if (typeid(std::string) == typeid(char))
				id = 1;
			else if (typeid(std::string) == typeid(short))
				id = 2;
			else if (typeid(std::string) == typeid(int))
				id = 3;
			else if (typeid(std::string) == typeid(long long) || typeid(std::string) == typeid(long))
				id = 4;
			else if (typeid(std::string) == typeid(float))
				id = 5;
			else if (typeid(std::string) == typeid(double))
				id = 6;
		}
		stringtag(const char* name) : stringtag() {
			this->name = name;
		}
		stringtag(std::string data) : stringtag() {
			this->data = data;
		}
		stringtag(std::string data, const char* name) : stringtag() {
			this->name = name;
			this->data = data;
		}
		stringtag(const char* name, std::string data) : stringtag() {
			this->name = name;
			this->data = data;
		}
		stringtag(tag* tag) {
			stringtag* t = dynamic_cast<stringtag*>(tag);
			this->data = t->data;
			this->id = t->id;
			this->name = t->name;
		}
		size_t load(char* bytes, size_t offset) {

			// Grabs the id of the tag
			id = bytes[offset];

			// Checks id against what the id of this tag type should be, and throw an exception if it's invalid
			if (id != 8 && id != (char)255) {
				throw invalid_tag_id_exception(id, 8);
				return 0;
			}

			// Get the length of the name
			unsigned short namelength = ((unsigned short)bytes[offset + 1] << 8) + (unsigned char)bytes[offset + 2];

			// Copy the name out of the bytes nbt data array
			name = new const char[namelength + 1];
			strncpy((char*)name, (const char*)&bytes[offset + 3], namelength);
			((char*)name)[namelength] = '\0';

			// Declare what will be the bytes of the tag's data
			unsigned short datlength = ((unsigned short)bytes[offset + 3 + namelength] << 8) + (unsigned char)bytes[offset + 4 + namelength];

			// Loop through the size of primitive type std::string, and set the equivelent byte of dat to the bytes of the nbt data
			for (unsigned short i = 0; i < datlength; i++)
				data += bytes[offset + 5 + i + namelength];

			//Return where the next tag will start in bytes[]
			return offset + 5 + datlength + namelength;

		}
		size_t write(char* buffer, size_t offset) {
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			strncpy(&buffer[offset + 3], name, namelength);

			// Convert the data into byte form, and copy it to the buffer
			strncpy(&buffer[offset + 3 + namelength], (const char*)data.c_str(), data.size());

			//Return where the next tag will start in buffer[]
			return offset + 3 + data.size() + namelength;
		}
		size_t write(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			for (char i = 0; i < namelength; i++)
				buffer.push_back(name[i]);

			// Convert the data into byte form, and copy it to the buffer
			for (char i = 0; i < data.size(); i++)
				buffer.push_back(data[i]);

			//Return where the next tag will start in buffer[]
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			//return (char*)data.c_str();
			std::vector<char> out = std::vector<char>();
			for (int i = 0; i < data.size(); i++) {
				out.push_back(data[i]);
			}
			return out;
		}

		// CONVERSION AND SYNstd::stringAX SIMPLIFICAstd::stringION
		operator std::string() {
			return data;
		}
		operator std::string* () {
			return &data;
		}
		operator stringtag* () {
			return this;
		}

#ifdef NBT_COMPILE
		std::string compilation(std::string regex) {
			std::string out = regex + "StringTag(" + std::string(name) + "): " + data + "\n";
			return out;
		}
#endif
	};


	class list : public tag {
	public:
		std::vector<tag*> tags;
		char tag_type = -1;
		list() {
			tags = std::vector<tag*>();
			id = 9;
		}
		list(std::vector<tag*> tags) {
			this->tags = tags;
			id = 9;
		}
		list(std::vector<tag*> tags, const char* name) {
			this->tags = tags;
			this->name = name;
			id = 9;
		}
		list(const char* name) {
			this->name = name;
			id = 9;
		}
		list(const char* name, std::vector<tag*> tags) {
			this->tags = tags;
			this->name = name;
			id = 9;
		}
		list(tag* tag) {
			list* t = dynamic_cast<list*>(tag);
			this->tags = t->tags;
			this->id = 9;
			this->tag_type = t->tag_type;
			this->name = t->name;
		}
		void clear() {
			this->tags.clear();
			name = "";
		}
		void discard();
		size_t load(char* bytes, size_t offset);
		size_t write(char* buffer, size_t offset) {
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			strncpy(&buffer[offset + 3], name, namelength);

			// Convert the length into byte form, and copy it to the buffer
			const char* len = reinterpret_cast<const char*>(tags.size());
			strncpy(&buffer[offset + 3 + namelength], len, sizeof(size_t));

			offset += 7 + namelength;
			for (int i = 0; i < tags.size(); i++)
				offset = tags[i]->write(buffer, offset);
			return offset;
		}
		size_t write(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			for (char i = 0; i < namelength; i++)
				buffer.push_back(name[i]);

			// Convert the data into byte form, and copy it to the buffer
			const char* len = reinterpret_cast<const char*>(tags.size());
			//strncpy(&buffer[offset + 3 + namelength], dat, sizeof(T));
			for (char i = 0; i < sizeof(size_t); i++)
				buffer.push_back(len[i]);

			for (int i = 0; i < tags.size(); i++)
				tags[i]->write(buffer);
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> buffer = std::vector<char>();
			for (int i = 0; i < tags.size(); i++)
				tags[i]->write(buffer);
			return buffer;
		}

		// SYNTAX AND CODE SIMPLIFICATION

		tag* operator[](size_t i) {
			return tags[i];
		}
		operator std::vector<tag*>() {
			return tags;
		}
		//operator tag** () {
		//	return &tags[0];
		//}
		void operator<<(tag* t) {
			if (tag_type == -1)
				tag_type = t->id;
			if (t->id != tag_type)
				throw illegal_list_tag_type(t->id);
			else
				tags.push_back(t);
		}
#ifdef NBT_COMPILE
		std::string compilation(std::string regex) {
			std::string out = regex + "ListTag(" + std::string(name) + "): " + std::to_string(tags.size()) + " tags {\n";
			for (int i = 0; i < tags.size(); i++) {
				std::string tag = tags[i]->compilation(regex + "\t");
				out += tag;

			}
			out += regex + "}\n";
			return out;
		}
#endif
	};

	class compound : public tag {
	public:
		std::vector<tag*> tags;
		compound() {
			tags = std::vector<tag*>();
			id = 10;
		}
		compound(std::vector<tag*> tags) {
			this->tags = tags;
			id = 10;
		}
		compound(std::vector<tag*> tags, const char* name) {
			this->tags = tags;
			this->name = name;
			id = 10;
		}
		compound(const char* name) {
			this->name = name;
			id = 10;
		}
		compound(const char* name, std::vector<tag*> tags) {
			this->tags = tags;
			this->name = name;
			id = 10;
		}
		compound(tag* tag) {
			compound* t = dynamic_cast<compound*>(tag);
			this->tags = t->tags;
			this->id = 10;
			this->name = t->name;
		}

		void discard() {
			for (int i = 0; i < tags.size(); i++) {
				switch (tags[i]->id) {
				case 7:
					((bytearray*)tags[i])->data.clear();
					break;
				case 8:
					((stringtag*)tags[i])->data.clear();
					break;
				case 10:
					((compound*)tags[i])->discard();
					break;
				case 9:
					((list*)tags[i])->discard();
					break;
				case 11:
					((intarray*)tags[i])->data.clear();
					break;
				case 12:
					((longarray*)tags[i])->data.clear();
					break;
				}
				delete tags[i];
			}
			tags.clear();
		}

		size_t load(char* bytes, size_t offset) {
			// Grabs the id of the tag

			//long long pick = 0, load = 0, push = 0;

			id = bytes[offset];
			if (id != 10 && id != (char)255) {
				throw invalid_tag_id_exception(id, 10);
				return 0;
			}

			// Get the length of the name
			unsigned short namelength = ((unsigned short)bytes[offset + 1] << 8) + (unsigned char)bytes[offset + 2];

			// Copy the name out of the bytes nbt data array
			name = new const char[namelength + 1];
			strncpy((char*)name, (const char*)&bytes[offset + 3], namelength);
			((char*)name)[namelength] = '\0';
			offset += 3 + namelength;
			//auto n = std::chrono::high_resolution_clock::now();
			char t;
			//long long total = 0;
			tag* tag;
			while (true) {
				t = bytes[offset];
				switch (t) {
				case 0:
					//tags.push_back(new end());
					//total = pick + load + push;
					//total /= 100;
					//if (total == 0) total = 1;
					//std::cout << "Compound tag: " << name << " : " << pick / total << "%, " << load / total << "%, " << push / total << "%" << std::endl;
					return offset + 1;
				case 1:
					tag = new bytetag();
					break;
				case 2:
					tag = new shorttag();
					break;
				case 3:
					tag = new inttag();
					break;
				case 4:
					tag = new longtag();
					break;
				case 5:
					tag = new floattag();
					break;
				case 6:
					tag = new doubletag();
					break;
				case 7:
					tag = new bytearray();
					break;
				case 8:
					tag = new stringtag();
					break;
				case 10:
					tag = new compound();
					break;
				case 9:
					tag = new list();
					break;
				case 11:
					tag = new intarray();
					break;
				case 12:
					tag = new longarray();
					break;
				default:
					throw missing_tag_id_exception(t);
					return 0;
				}
				//pick += (std::chrono::high_resolution_clock::now() - n).count();
				//n = std::chrono::high_resolution_clock::now();
				offset = tag->load(bytes, offset);
				//load += (std::chrono::high_resolution_clock::now() - n).count();
				//n = std::chrono::high_resolution_clock::now();
				tags.push_back(tag);
				//push += (std::chrono::high_resolution_clock::now() - n).count();
				//n = std::chrono::high_resolution_clock::now();
			}
		}
		size_t write(char* buffer, size_t offset) {
			// Push the id of the tag
			buffer[offset] = id;

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer[offset + 1] = (namelength >> 8) & 0xff;
			buffer[offset + 2] = (namelength) & 0xff;

			// Copy the name into the buffer
			strncpy(&buffer[offset + 3], name, namelength);

			offset += 3 + namelength;
			for (int i = 0; i < tags.size(); i++)
				offset = tags[i]->write(buffer, offset);
			return offset;
		}
		size_t write(std::vector<char>& buffer) {
			// Push the id of the tag
			buffer.push_back(id);

			// Grab the name length and store it in the buffer
			unsigned short namelength = strlen(name);
			buffer.push_back((namelength >> 8) & 0xff);
			buffer.push_back((namelength) & 0xff);

			// Copy the name into the buffer
			for (char i = 0; i < namelength; i++)
				buffer.push_back(name[i]);

			for (int i = 0; i < tags.size(); i++)
				tags[i]->write(buffer);
			return buffer.size();
		}
		std::vector<char> value_bytes() {
			std::vector<char> buffer = std::vector<char>();
			for (int i = 0; i < tags.size(); i++)
				tags[i]->write(buffer);
			return buffer;
		}

		tag* get(const char* name) {
			for (int i = 0; i < tags.size(); i++)
				if (std::string(tags[i]->name) == std::string(name))
					return tags[i];
			return nullptr;
		}

		// SYNTAX AND CODE SIMPLIFICATION

		tag* operator[](const char* name) {
			for (int i = 0; i < tags.size(); i++)
				if (std::string(tags[i]->name) == std::string(name))
					return tags[i];
			return nullptr;
		}
		operator std::vector<tag*>() {
			return tags;
		}
		operator tag** () {
			return &tags[0];
		}
		void operator<<(tag* t) {
			tags.push_back(t);
		}
		//void operator<<(bytetag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(shorttag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(inttag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(longtag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(floattag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(doubletag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(bytearray t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(stringtag t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(compound t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(list t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(intarray t) {
		//	tags.push_back(&t);
		//}
		//void operator<<(longarray t) {
		//	tags.push_back(&t);
		//}
		size_t size() {
			return tags.size();
		}

#ifdef NBT_COMPILE
		std::string compilation(std::string regex) {
			std::string out = regex + "CompoundTag(" + std::string(name) + "): " + std::to_string(tags.size()) + " tags {\n";
			for (int i = 0; i < tags.size(); i++) {
				std::string tag = tags[i]->compilation(regex + "\t");
				out += tag;

			}
			out += regex + "}\n";
			return out;
		}
#endif
	};

#ifdef NBT_SHORTHAND
	typedef bytetag bt;
	typedef shorttag st;
	typedef inttag it;
	typedef longtag lt;
	typedef floattag ft;
	typedef doubletag dt;
	typedef bytearray ba;
	typedef list li;
	typedef compound c;
	typedef stringtag str;
	typedef intarray ia;
	typedef longarray la;
#endif
}
#endif