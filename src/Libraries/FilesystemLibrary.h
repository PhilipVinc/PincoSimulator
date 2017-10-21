//
// Created by Filippo Vicentini on 21/10/17.
//
// This is a small header to include either std::filesystem, std::experimental::filesystem
// or boost::filesystem (in the order).
//

#ifndef SIMULATOR_FILESYSTEMLIBRARY_H
#define SIMULATOR_FILESYSTEMLIBRARY_H

// __has_include is a c++17 feature, therefore we need to check if the compiler supports it.
// If the compiler is not supporting it, it surely won't support std::filesystem even, and
// we will have to fall back on boost.
#ifdef __has_include
	// Here I am just future proofing for when the experimental feature will get out of experimental
	#if __has_include(<filesystem>)
		#include <filesystem>

	#elif __has_include(<experimental/filesystem>)
		#include <experimental/filesystem>
		namespace filesystem = std::experimental::filesystem;

	#elif __has_include(<boost/filesystem.hpp>)
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wdocumentation"
		#include <boost/filesystem.hpp>
		#pragma clang pop
		namespace filesystem = boost::filesystem;
	#else
		#error "No Filesystem library found."
	#endif
#else
	#warning "__has_include not defined. Falling back to boost."
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdocumentation"
	#include <boost/filesystem.hpp>
	#pragma clang pop
	namespace filesystem = boost::filesystem;
#endif
#endif //SIMULATOR_FILESYSTEMLIBRARY_H
