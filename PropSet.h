#pragma once

#include <iostream>
#include <vector>

#include <boost/log/trivial.hpp>

#include "conote.h"

using namespace std;

namespace conote
{
namespace onestore
{

class PropSet;


// I'm 18 and drinking is OK.
// This union will not have dangling pointers unless it has been copied after the PropSet instance(which created this
// union) is destroyed.
// Why you class this and all others C like? I'm doing this only for PropSet
typedef union _PropSetVal
{
    uint8_t oneByteData;
    uint16_t twoByteData;
    uint32_t fourByteData;
    uint64_t eightByteData;
    OneStoreprtFourBytesOfLengthFollowedByData* fourBytesFollwoedByData;
    OneStoreprtArrayOfPropertyValues* arrayOfPropValues;
    std::vector<CompactID>* ids;
    PropSet* childPropSet;
} PropSetVal;

typedef struct _prtArrayOfPropertyValues {
	OneStorePropertyID prid;
	vector< PropSetVal > propSetVals;
} prtArrayOfPropertyValues;

class PropSet
{
public:
    PropSet();
    ~PropSet();
    uint16_t get_properties_count() const
    {
	return cProperties;
    }
    const vector<PropSetVal>& get_data() const
    {
	return data;
    }
    const vector<OneStorePropertyID>& get_prids() const
    {
	return prids;
    }
	void parse_propset_from_stream(FILE* file, uint64_t objectSpaceObjPropSetRef, uint64_t propSetOffset, vector<CompactID> oids,
		vector<CompactID> osids,
		vector<CompactID> ctxIds);

private:

    uint16_t cProperties;
    /* this vector refers some of the items in the below pools using pointers thru union pointer fields */
    vector<OneStorePropertyID> prids;
    vector<PropSetVal> data;

    vector<vector<CompactID > > idsVectorPool;
    vector<vector<uint8_t > > byteStreamsPool;
    vector<OneStoreprtArrayOfPropertyValues> arrayOfPropsPool;
	
};

}
}
