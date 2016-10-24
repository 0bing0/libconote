/*
 * C like interface to interact with the OneStore revision file. I had to write the C++ version first for the sake of clean code and I have to understand
 * the overall things going on, how the chunks are organized.
 * I'll think about the performance and C version of the library later.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <boost/log/trivial.hpp>

#include "conote.h"
#include "PropSet.h"

using namespace std;

namespace conote
{
	struct OneStoreFCR : OneStoreFileChunkReference64x32 {
		std::string to_string();
	};
	
class OneStore
{
private:
	OneStore(const OneStore& rhs);
	OneStore& operator=(const OneStore& rhs);

public:
	static OneStoreFileHeaderRaw read_file_header(const std::string& fileName);
	static OneStoreFileHeaderRaw read_file_header(FILE* file);
	static OneStoreFileNodeListFragment read_file_node_list_fragment(FILE* file, uint64_t stp, uint64_t& errorCode);
	/* recursive unset - populates only the filenodeheader and stp of each filenode struct
	 * recursive set - fetches all the inner contents of filenodedata including, filenodereference and filenodedata struct
	 */
	static vector<OneStoreFileNode> get_file_nodes(FILE* file, OneStoreFileNodeListFragment& frag, uint64_t fragmentStp, uint32_t fragCb, uint recursiveFlag);
	/* the fileNode->fnd must have the actual data
	 */
	static OneStoreFileNodeChunkReference get_actual_file_node_chunk_reference(FILE* file, OneStoreFileNode& fileNode);
	static OneStoreFileNodeData get_actual_file_node_struct(FILE* file, OneStoreFileNode& fileNode);
	

	/* the below methds should be called sequentially to know the stream start of each struct members
	 */
 
	static vector<CompactID> get_object_space_object_oids(FILE* file, 
		OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, 
		OneStoreObjectSpaceObjectPropSet &emptyPropSet,
		OneStoreObjectSpaceObjectStreamHeader &emptyHeader);

	static vector<CompactID> get_object_space_object_osids(FILE* file, 
		OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, 
		uint64_t memberStpOffset, 
		OneStoreObjectSpaceObjectPropSet &propSet,
		OneStoreObjectSpaceObjectStreamHeader &emptyHeader);
		
	static vector<CompactID> get_object_space_object_contextids(FILE* file, 
		OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, 
		uint64_t memberStpOffset, 
		OneStoreObjectSpaceObjectPropSet &propSet,
		OneStoreObjectSpaceObjectStreamHeader &emptyHeader);
		
	/* returns the props count, the vectors and a class:PropSet, 
	 * this is important, the PropSet object is needed to
	 * fetch the values in rgData vector
	 */
	static void get_propset_fields(FILE* file, 
		uint64_t objectSpaceObjPropSetRef,
		uint64_t propSetOffset,
		size_t &outPropertiesCount,
		vector<OneStorePropertyID> &rgPrids,
		vector<conote::onestore::PropSetVal> &rgData,
		conote::onestore::PropSet &propSetRefObj,
		vector<CompactID> oids,
		vector<CompactID> osids,
		vector<CompactID> ctxIds);
		
	
	static JCID get_jcid_by_jci(uint jci);
	static uint32_t jcid_to_uint32(JCID jcid);
	
	
	static void print_guid(GUID guid);
	static void print_extended_guid(ExtendedGUID eguid);

private:
	OneStore();
	~OneStore();
	static uint64_t get_footer_magic_number_index(uint64_t fileNodeListHeaderStart, FILE* file);
	static void get_next_fragment_ref(FILE* file, OneStoreFileNodeListFragment &frag);
	static void copy_fnd_to_struct(uint8_t* fnd, void* structRef, size_t structSize, void** outFnd, size_t* outStructSize, OneStoreFileNode& fileNode);
	
	static vector<CompactID> get_object_space_object_some_ids(FILE* file, 
		OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, 
		uint64_t memberStpOffset,
		uint64_t& outNextIDsOffset,
		OneStoreObjectSpaceObjectStreamHeader &emptyHeader); 

};

}

