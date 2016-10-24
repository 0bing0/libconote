#include "PropSet.h"

conote::onestore::PropSet::PropSet()
{
}

conote::onestore::PropSet::~PropSet()
{
}

void conote::onestore::PropSet::parse_propset_from_stream(FILE* file, uint64_t objectSpaceObjPropSetRefStp, uint64_t propSetOffset, vector<CompactID> oids,
		vector<CompactID> osids,
		vector<CompactID> ctxIds)
{
	uint64_t rgDataOffset = 0, rgDataPtr = 0, oidCtr = 0, osidCtr = 0, ctxIdCtr = 0, pridsPtr = 0;
	
	fseek(file, objectSpaceObjPropSetRefStp + propSetOffset, SEEK_SET);
	
	fread(&this->cProperties, sizeof(uint16_t), 1, file);
	
	// the startin of the PropSet field + size of cProperties field in PropSet + (cProperties * size of PropertyID structs)
	rgDataOffset = propSetOffset + sizeof(uint16_t) + (this->cProperties * sizeof(OneStorePropertyID));
	rgDataPtr = objectSpaceObjPropSetRefStp + rgDataOffset;
	pridsPtr = objectSpaceObjPropSetRefStp + propSetOffset + sizeof(uint16_t);
	
	for(int i = 0; i < this->cProperties; i++) {
		OneStorePropertyID id;
		
		fseek(file, pridsPtr + (i * sizeof(OneStorePropertyID)), SEEK_SET);
		fread(&id, sizeof(OneStorePropertyID), 1, file);
		
#ifndef RELEASE
		uint32_t idint;
		fseek(file, pridsPtr + (i * sizeof(OneStorePropertyID)), SEEK_SET);
		fread(&idint, sizeof(uint32_t), 1, file);
		BOOST_LOG_TRIVIAL(debug) << "PropertyID val: " << std::hex << idint << std::dec << endl;
#endif
	
		this->prids.push_back(id);
		
		fseek(file, rgDataPtr, SEEK_SET);
		
		PropSetVal val;
		
		// read the bytes and size
		switch(id.type) {
			case PROPERTYID_NODATA: {
				break;
			}
			case PROPERTYID_BOOL: {
				break;
			}
			case PROPERTYID_ONEBYTEOFDATA: {
				fread(&val.oneByteData, sizeof(uint8_t), 1, file);
				rgDataPtr += sizeof(uint8_t);
				break;
			}
			case PROPERTYID_TWOBYTESOFDATA: {
				fread(&val.twoByteData, sizeof(uint16_t), 1, file);
				rgDataPtr += sizeof(uint16_t);
				break;
			}
			case PROPERTYID_FOURBYTESOFDATA: {
				fread(&val.fourByteData, sizeof(uint32_t), 1, file);
				rgDataPtr += sizeof(uint32_t);
				BOOST_LOG_TRIVIAL(debug) << "Four bytes val: " << std::hex << val.fourByteData << std::dec << endl;
				break;
			}
			case PROPERTYID_EIGHTBYTESOFDATA: {
				fread(&val.eightByteData, sizeof(uint64_t), 1, file);
				rgDataPtr += sizeof(uint64_t);
				break;
			}
			case PROPERTYID_FOURBYTESOFLENGTHFOLLOWEDBYDATA: {
				uint32_t arraySize;
				uint8_t byte;
				
				fread(&arraySize, sizeof(uint32_t), 1, file);
				rgDataPtr += sizeof(uint32_t);
				
				vector < uint8_t > array;
				
				std::cout << "dumping array: ";
				for(uint32_t j = 0; j < arraySize; j++ ) {
					
					fread(&byte, sizeof(uint8_t), 1, file);
					rgDataPtr += sizeof(uint8_t);
					std::cout << (char) byte;
					array.push_back(byte);
				}
				
				std::cout << endl;
				
				byteStreamsPool.push_back(array);
				
				break;
			}
			case PROPERTYID_OBJECTID:
			case PROPERTYID_CONTEXTID:
			case PROPERTYID_OBJECTSPACEID: {
				vector<CompactID> ids;
				switch(id.type) {
					case PROPERTYID_OBJECTID:
						ids.push_back(oids[oidCtr++]);
						break;
					case PROPERTYID_CONTEXTID:
						ids.push_back(osids[osidCtr++]);
						break;
					case PROPERTYID_OBJECTSPACEID:
						ids.push_back(ctxIds[ctxIdCtr++]);
						break;
				}
				
				this->idsVectorPool.push_back(ids);
				break;
				
			}
			case PROPERTYID_ARRAYOFOBJECTIDS:
			case PROPERTYID_ARRAYOFOBJECTSPACEIDS:
			case PROPERTYID_ARRAYOFCONTEXTIDS: {
				// read the first 4 byte integer to know the size of array
				
				vector<CompactID> ids;
				uint32_t idCount;
				
				fread(&idCount, sizeof(uint32_t), 1, file);
				rgDataPtr += sizeof(uint32_t);
				
				for(int j = 0; j < idCount; j++) {
					switch(id.type) {
						case PROPERTYID_ARRAYOFOBJECTIDS:
							ids.push_back(oids[oidCtr++]);
							break;
						case PROPERTYID_ARRAYOFOBJECTSPACEIDS:
							ids.push_back(osids[osidCtr++]);
							break;
						case PROPERTYID_ARRAYOFCONTEXTIDS:
							ids.push_back(ctxIds[ctxIdCtr++]);
							break;
					}
				}
				this->idsVectorPool.push_back(ids);
				break;
			}
			case PROPERTYID_ARRAYOFPROPERTYVALUES: {
				break;
			}
			case PROPERTYID_PROPERTYSET: {
				break;
			}
		}
		
		
		// increment the rgDataPtr with no. of bytes read
	}
	
}
