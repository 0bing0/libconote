#ifndef ONENOTETOCINFO_H
#define ONENOTETOCINFO_H

#include "conote.h"
#include "OneStore.h"
#include "PropSet.h"
#include "json_serializer.hpp"

#include <boost/log/trivial.hpp>

#include <vector>
#include <stack>
#include <string>

using namespace std;

BOOST_FUSION_ADAPT_STRUCT(::OneStoreFileChunkReference64x32, (uint64_t, stp) (uint32_t, cb))

namespace conote
{

class OneNoteTOCInfo
{
private:
	FILE* tocFile;
	string tocFileName;
	OneStoreFileHeaderRaw header;
	OneStoreFileNodeListFragment rootFnlFragment, rootObjectManifestListFragment;
	
	vector<uint64_t> visitedStps;

	OneNoteTOCInfo();
	OneNoteTOCInfo(const OneNoteTOCInfo& rhs);
	OneNoteTOCInfo& operator=(const OneNoteTOCInfo& rhs);
	void parseFile();
	void parse_frag(uint64_t fragStp, uint32_t fragCb);

public:
	OneNoteTOCInfo(const char* fileName);
	void initialize();
	vector<string> getAllSectionFileNames();
	~OneNoteTOCInfo();

};

}

#endif // ONENOTETOCINFO_H
