#pragma once

#include <iostream>
#ifndef FILECHOOSER
#define FILECHOOSER

#include <ShlObj_core.h>
#include <atlstr.h>
#include <Windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ios>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include <assert.h>

#include <zstr-master/src/zstr.hpp>
#include "boost/iostreams/stream_buffer.hpp"

static std::string ChooseOpenFile(const char* name, const char* filters) {
	char prePathB[MAX_PATH];
	std::string prePath = std::string(prePathB, GetCurrentDirectoryA(MAX_PATH, prePathB));
	char filename[ MAX_PATH ];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = LPCWSTR(filters);
	ofn.lpstrFile = LPWSTR(filename);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = LPCWSTR(name);
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(LPOPENFILENAMEA(&ofn)))
	{
		std::cout << "You chose the file \"" << filename << "\" " << SetCurrentDirectoryA(prePath.c_str()) << "\n";
		return std::string(filename);
	}
	else
	{
		// All this stuff below is to tell you exactly how you messed up above. 
		// Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
		switch (CommDlgExtendedError())
		{
		case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
		case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
		case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
		case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
		case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
		case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
		case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
		case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
		case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
		case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
		case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
		case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
		case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
		case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
		case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
		default: std::cout << "You cancelled.\n";
		}
	}

	SetCurrentDirectoryA(prePath.c_str());

	return "NONE";
}

static std::string ChooseOpenFolder(const char* name) {
	IFileDialog* p;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p)))) {
		DWORD dwOptions;
		if (SUCCEEDED(p->GetOptions(&dwOptions))) {
			p->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}
		if (SUCCEEDED(p->Show(NULL))) {
			IShellItem* i;
			if (SUCCEEDED(p->GetResult(&i))) {
				LPWSTR path;
				if (!SUCCEEDED(i->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path))) {
					MessageBox(NULL, L"GetIDListName() failed", NULL, NULL);
					return "NONE";
				}
				return std::string(CW2A(path));
				i->Release();
			}
		}
	}

	return "NONE";
}

static std::string ChooseSaveFile(const char* title, const char* filters) {
	char prePathB[MAX_PATH];
	std::string prePath = std::string(prePathB, GetCurrentDirectoryA(MAX_PATH, prePathB));
	char filename[ MAX_PATH ];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = LPCWSTR(filters);
	ofn.lpstrFile = LPWSTR(filename);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = LPCWSTR(title);
	ofn.Flags = OFN_DONTADDTORECENT | OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT;

	if (GetSaveFileNameA(LPOPENFILENAMEA(&ofn)))
	{
		std::cout << "You chose the file \"" << filename << "\" "<< SetCurrentDirectoryA(prePath.c_str())<<"\n";
		return std::string(filename);
	}
	else
	{
		// All this stuff below is to tell you exactly how you messed up above. 
		// Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
		switch (CommDlgExtendedError())
		{
		case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
		case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
		case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
		case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
		case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
		case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
		case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
		case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
		case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
		case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
		case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
		case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
		case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
		case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
		case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
		default: std::cout << "You cancelled.\n";
		}
	}

	return "NONE";
}

static const char* garunteeExtension(const std::string &path, const std::string &extension) {
	bool has = path.size() >= extension.size() &&
		path.compare(path.size() - extension.size(), extension.size(), extension) == 0;
	if (has)
		return path.c_str();
	else return (path + extension).c_str();
}

static const char* readFile(const char* filepath) {
	std::ifstream f;
	f.open(filepath);
	if (f.is_open()) {
		std::string line;
		std::string output;
		while (getline(f, line)) {
			output += line + "\n";
			std::cout << line << std::endl;
		}
		
		f.close();
		return output.c_str();
	}
	else {
		f.close();
		std::cout << "ERROR! Couldn't load file \"" << filepath << "\"!" << std::endl;
		return "ERROR";
	}
}

static std::string ToHex(const std::string& s, bool upper_case)
{
	std::ostringstream ret;

	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		int z = s[i] & 0xff;
		ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << z;
		//std::cout << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << z << " ";
	}
	std::cout << std::endl;

	return ret.str();
}

static const std::string readBinary(const char* filepath) {
	std::ifstream f(filepath, std::ios::in|std::ios::binary|std::ios::ate);
	if (f.is_open()) {
		std::ifstream::pos_type size = f.tellg();
		char* output = new char[size];
		f.seekg(0, std::ios::beg);
		f.read(output, size);
		f.close();
		std::string s = ToHex(std::string(output, size), true);
		return s;
	}
	else {
		f.close();
		std::cout << "ERROR! Couldn't load file \"" << filepath << "\"!" << std::endl;
		return "ERROR";
	} 
}

static const std::vector<unsigned char> readAsBytes(const char* filepath, bool compressed = false, size_t preLength = 1<<24) {	

	std::vector<unsigned char> bytes = std::vector<unsigned char>();

	if (compressed) {
		zstr::ifstream infile(filepath, std::ios_base::binary);
		
		//infile.seekg(0, std::ios::end);
		size_t length = preLength;
		//infile.seekg(0, std::ios::beg);
		std::vector<unsigned char> chars = std::vector<unsigned char>(length);

		infile.read((char*)&chars[0], length);
		std::cout << chars.size() << " : " << length << std::endl;
		return chars;
	}
	else {
		std::ifstream infile(filepath, std::ios_base::binary);

		// File size
		infile.seekg(0, std::ios::end);
		size_t length = infile.tellg();
		infile.seekg(0, std::ios::beg);
		std::vector<char> chars = std::vector<char>(length);

		// Read
		infile.read(&chars.data()[0], length);

		for (int i = 0; i < chars.size(); i++) {
			bytes.push_back((unsigned char)chars[i]);
		}
	}

	return bytes;
}

static const std::vector<unsigned char> decompress(std::vector<unsigned char> data) {
	namespace bio = boost::iostreams;
	//bio::stream_buffer<bio::array_source> stream_buffer((const char*)&data[0], data.size());
	//zstr::istreambuf converter = zstr::istreambuf(&stream_buffer);
	//zstr::istream* conv = &zstr::istream(&converter);
	std::vector<unsigned char> out = std::vector<unsigned char>();
	//while (*conv)
	//	out.push_back(unsigned char(conv->get()));

	return out;
}

static unsigned char* decompress(unsigned char* data, size_t length, size_t outlength = 1 << 20) {
	namespace bio = boost::iostreams;
	bio::stream_buffer<bio::array_source> stream_buffer((const char*)data, length);
	zstr::istreambuf converter = zstr::istreambuf((std::streambuf*)&stream_buffer, length, true);
	std::istream conv(&converter);
	conv.exceptions(std::ios_base::badbit);
	unsigned char* buff = new unsigned char[outlength];
	while (true) {
		conv.read((char*)buff, outlength);
		std::streamsize cnt = conv.gcount();
		if (cnt == 0) break;
	}

	return buff;
}

static void writeBinary(const char* filepath, unsigned char* bytes, int size) {
	std::ofstream f(filepath, std::ios::out | std::ios::binary);
	std::cout << "FILE: " << filepath << " Size: " << size << std::endl;
	if (f.is_open()) {
		for(int i = 0; i<size; i++)
			f << bytes[i];
		f.close();
	}
	else {
		f.close();
		std::cout << "ERROR! Couldn't load file \"" << filepath << "\"!" << std::endl;
	}
}

static void writeFile(const char* filepath, std::string data) {
	std::ofstream f(filepath, std::ios::out);
	std::cout << "FILE: " << filepath << " Size: " << data.size() << std::endl;
	if (f.is_open()) {
		//for (int i = 0; i < size; i++)
			f << data;
		f.close();
	}
	else {
		f.close();
		std::cout << "ERROR! Couldn't load file \"" << filepath << "\"!" << std::endl;
	}
}


static std::vector<unsigned char> hex(const std::string &data) {
	std::vector<unsigned char> bytes = std::vector<unsigned char>();
	for (int i = 0; i < data.length(); i += 2) {
		std::stringstream ss;
		ss << std::hex << std::setw(2) << (data.at(i)) << data.at(i + 1);
		bytes.push_back((unsigned char)std::stoi(ss.str(), 0, 16));
	}
	std::cout << std::endl;
	return bytes;
}

static std::vector<unsigned char> bytes(const std::string &data) {
	std::vector<unsigned char> bytes = std::vector<unsigned char>();
	for (int i = 0; i < data.length(); i++)
		bytes.push_back(data.at(i));
	return bytes;
}

static unsigned char hexval(unsigned char c) {
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	else if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	else abort();
}

static void hex2ascii(const std::string& in, std::string& out) {
	out.clear();
	out.reserve(in.length() / 2);
	for (std::string::const_iterator p = in.begin(); p != in.end(); p++) {
		unsigned char c = hexval(*p);
		p++;
		if (p == in.end()) break;
		c = (c << 4) + hexval(*p);
		out.push_back(c);
	}
}

#endif