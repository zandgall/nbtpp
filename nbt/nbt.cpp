#include "nbt.hpp"

size_t nbt::list::load(char* bytes, size_t offset){
	// Grabs the id of the tag
	id = bytes[offset];
	if (id != 9 && id != (char)255) {
		throw invalid_tag_id_exception(id, 9);
		return 0;
	}

	// Get the length of the name
	unsigned short namelength = ((unsigned short)bytes[offset + 1] << 8) + ((unsigned char)bytes[offset + 2]);
	name = new const char[namelength + 1];
	strncpy((char*)name, (const char*)&bytes[offset + 3], namelength);
	((char*)name)[namelength] = '\0';
	// Get the amount of elements in the array
	tag_type = bytes[offset + 3 + namelength];
	unsigned int length = ((unsigned int)bytes[offset + 4 + namelength] << 24) + ((unsigned int)bytes[offset + 5 + namelength] << 16) + ((unsigned short)bytes[offset + 6 + namelength] << 8) + ((unsigned char)bytes[offset + 7 + namelength]);

	offset += 8 + namelength;
	tag* tag;
	for (unsigned int i = 0; i < length; i++) {
		switch (tag_type) {
		case 0:
			throw illegal_list_tag_type(tag_type);
			return 0;
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
			throw missing_tag_id_exception(tag_type);
			return 0;
		}
		bytes[offset - 3] = 255;
		bytes[offset - 2] = '\0';
		bytes[offset - 1] = '\0';
		offset = tag->load(bytes, offset - 3);

		tags.push_back(tag);
	}
	return offset;
}

void nbt::list::discard() {
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