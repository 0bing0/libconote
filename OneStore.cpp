#include "OneStore.h"

conote::OneStore::OneStore()
{
}

conote::OneStore::~OneStore()
{
}

conote::OneStore::OneStore(const OneStore& rhs)
{
}

std::string conote::OneStoreFCR::to_string() {
	stringstream s;
	s << this->cb << "," << this->stp;
	return s.str();
}


OneStoreFileHeaderRaw conote::OneStore::read_file_header(const std::string& fileName)
{
	FILE* file;
	file = fopen(fileName.c_str(), "r");
	OneStoreFileHeaderRaw header = read_file_header(file);
	fclose(file);
	return header;
}

OneStoreFileHeaderRaw conote::OneStore::read_file_header(FILE* file)
{
	OneStoreFileHeaderRaw header;
	fread(&header, sizeof(OneStoreFileHeaderRaw), 1, file);
	return header;
}

OneStoreFileNodeListFragment conote::OneStore::read_file_node_list_fragment(FILE* file, uint64_t stp, uint64_t& errorCode)
{
	OneStoreFileNodeListFragment frag;
	
	BOOST_LOG_TRIVIAL(debug) << "FNL Stp: " << stp;
	
	if(file == NULL)
		return frag;
	
	uint64_t fragStp = stp;
	uint64_t footerIndex = 0;
	
	fseek(file, fragStp, SEEK_SET);
	OneStoreFileNodeListHeader fileNodeListHeader;
	fread(&fileNodeListHeader, sizeof(OneStoreFileNodeListHeader), 1, file);
	
	if(fileNodeListHeader.uintMagic != FILENODELISTFRAGMENT_HEADER_MAGIC_NUMBER)
		return frag;
	
	footerIndex = get_footer_magic_number_index(fragStp, file);
	frag.extra.footerMagicNumberStartIndex = footerIndex;
	
	frag.header = fileNodeListHeader;
	
	// get next fragment
	get_next_fragment_ref(file, frag);
	
	//TODO: Complete this method
	
	return frag;
}

OneStoreFileNodeChunkReference conote::OneStore::get_actual_file_node_chunk_reference(FILE* file, OneStoreFileNode& fileNode)
{
	// TODO: handle empty struct
	
	OneStoreFileNodeChunkReference ref = { SET_UINT64, 0 };
	if(file == NULL)
		return ref;
	
	size_t stpBytes = 0, cbBytes = 0, fileNodeChunkRefSize = 0;
	uint64_t readStp = SET_UINT64, readCb = SET_UINT64;
	uint8_t bytes[16];
	uint8_t uncompressFlag = 0, setFlag = 1;
	uint64_t stp = 0, cb = 0;
	
	fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
	fread(&bytes, sizeof(uint8_t), fileNode.header.Size < 16 ? fileNode.header.Size : 16, file);
	
	switch(fileNode.header.cBaseType) {
		case FILENODE_BASETYPE_REF:
		case FILENODE_BASETYPE_REF_FILE_NODE_LIST:
			switch(fileNode.header.aStpFormat) {
				case FILENODE_STPFORMAT_COMPRESSED_2_BYTES:
					stpBytes = 2;
					uncompressFlag = 1;
					break;
				case FILENODE_STPFORMAT_COMPRESSED_4_BYTES:
					stpBytes = 4;
					uncompressFlag = 1;
					break;
				case FILENODE_STPFORMAT_UNCOMPRESSED_4_BYTES:
					stpBytes = 4;
					break;
				case FILENODE_STPFORMAT_UNCOMPRESSED_8_BYTES:
					stpBytes = 8;
					break;
			}
			
			/* because the filenodechunkref is the first struct; the fields
			 * stp and then cb can be read
			 */
			for(int i = 0; i < stpBytes; i++) {
				stp |= (uint64_t)(bytes[i] << (8 * i));
				if(bytes[i] != 0xff)
					setFlag = 0;
			}

			if(uncompressFlag) {
				stp *= 8;
				uncompressFlag = 0;
			}
			
			if(setFlag)
				stp = SET_UINT64;
			
			switch(fileNode.header.bCbFormat) {
				case FILENODE_CBFORMAT_COMPRESSED_1_BYTE:
					cbBytes = 1;
					uncompressFlag = 1;
					break;
				case FILENODE_CBFORMAT_COMPRESSED_2_BYTES:
					cbBytes = 2;
					uncompressFlag = 1;
					break;
				case FILENODE_CBFORMAT_UNCOMPRESSED_4_BYTES:
					cbBytes = 4;
					break;
				case FILENODE_CBFORMAT_UNCOMPRESSED_8_BYTES:
					cbBytes = 8;
					break;
			}
			
			for(int i = 0; i < cbBytes; i++) {
				cb |= (uint64_t)(bytes[stpBytes + i] << (8 * i));
			}
			
			if(uncompressFlag)
				cb *= 8;
			break;
	}
	
	ref.stp = stp;
	ref.cb = cb;
	fileNode.extra.fileNodeChunkRefSize = stpBytes + cbBytes;
	
	return ref;
}

uint64_t conote::OneStore::get_footer_magic_number_index(uint64_t fileNodeListHeaderStart, FILE* file)
{
	uint64_t outIndex = 0xFFFFFFFFFFFFFFFF;
	if(file == NULL)
		return outIndex;
	
	uint64_t magicNum = FILENODELISTFRAGMENT_FOOTER_MAGIC_NUMBER;
	long int seekPos = fileNodeListHeaderStart;
	uint64_t readNum = 0;
	size_t fileSize = 0;
	
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
		
	while(1) {
		fseek(file, seekPos++, SEEK_SET);
		fread(&readNum, sizeof(uint64_t), 1, file);
		
		if(readNum == magicNum) {
			outIndex = seekPos - 1;
			break;
		}
		
		if(seekPos >= fileSize) {
			break;
		}
	}
	
	return outIndex;
}


OneStoreFileNodeData conote::OneStore::get_actual_file_node_struct(FILE* file, OneStoreFileNode& fileNode)
{
	OneStoreFileNodeData fnd;
	fileNode.extra.refFlag = 0;
	fileNode.extra.otherRefFlag = 0;
	switch((uint32_t)fileNode.header.fileNodeID) {
		case FILENODEID_OBJECTSPACEMANIFESTLISTREFERENCEFND: {
			fnd.objectSpaceManifestListReferenceFND.ref = get_actual_file_node_chunk_reference(file, fileNode);
			fileNode.extra.refFlag = 1;
			/* [ File Node Header ][[ ref:FileNodeChunkRef ][ gosid:ExtendedGUID ]] */
			/*											=======^					*/
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader) + fileNode.extra.fileNodeChunkRefSize, SEEK_SET);
			fread(&fnd.objectSpaceManifestListReferenceFND.gosid, sizeof(ExtendedGUID), 1, file);
			
			cout << "FileNodeID: objectSpaceManifestListReferenceFND:" << endl;
			print_extended_guid(fnd.objectSpaceManifestListReferenceFND.gosid);
			cout << endl;

			break;
		}
		case FILENODEID_OBJECTSPACEMANIFESTROOTFND: {
			/* [ File Node Header ][[ gosid:ExtendedGUID ]] */
			/*					=======^					*/
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.objectSpaceManifestRootFND.gosidRoot, sizeof(ExtendedGUID), 1, file);
			
			cout << "FileNodeID: objectSpaceManifestRootFND:" << endl;
			print_extended_guid(fnd.objectSpaceManifestRootFND.gosidRoot);
			cout << endl;
			
			break;
		}
		case FILENODEID_OBJECTSPACEMANIFESTLISTSTARTFND: {
			/* [ File Node Header ][[ gosid:ExtendedGUID ]] */
			/*					=======^					*/
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.objectSpaceManifestListStartFND.gosid, sizeof(ExtendedGUID), 1, file);
			break;
		}
		case FILENODEID_REVISIONMANIFESTLISTREFERENCEFND: {
			fnd.revisionManifestListReferenceFND.ref = get_actual_file_node_chunk_reference(file, fileNode);
			fileNode.extra.refFlag = 1;
			/* [ File Node Header ][[ ref:FileNodeChunkRef ]] */
			/*					=======^					  */
			break;
		}
		case FILENODEID_REVISIONMANIFESTLISTSTARTFND: {
			/* [ File Node Header ][ RevisionManifestListStartFND ] */
			/*					=======^							  */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.revisionManifestListStartFND, sizeof(OneStoreRevisionManifestListStartFND), 1, file);
			
			cout << "FileNodeID: revisionManifestListStartFND:" << endl;
			print_extended_guid(fnd.revisionManifestListStartFND.gosid);
			cout << endl;
			
			break;
		}
		case FILENODEID_REVISIONMANIFESTSTART4FND: {
			/* [ File Node Header ][ RevisionManifestStart4FND ] */
			/*					=======^						 */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.revisionManifestStart4FND, sizeof(OneStoreRevisionManifestStart4FND), 1, file);
			
			cout << "FileNodeID: revisionManifestStart4FND:" << endl;
			print_extended_guid(fnd.revisionManifestStart4FND.rid);
			cout << "\tDep: " << endl;
			print_extended_guid(fnd.revisionManifestStart4FND.rid);
			cout << "\tRevRole: " << fnd.revisionManifestStart4FND.RevisionRole;
			cout << endl;
			
			break;
		}
		case FILENODEID_GLOBALIDTABLESTARTFNDX: {
			/* [ File Node Header ][ OneStoreGlobalIdTableStartFNDX ] */
			/*					=======^							  */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.globalIdTableStartFNDX, sizeof(OneStoreGlobalIdTableStartFNDX), 1, file);

			break;
		}
		case FILENODEID_GLOBALIDTABLEENTRYFNDX: {
			/* [ File Node Header ][ GlobalIdTableEntryFNDX ] */
			/*					=======^					  */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.globalIdTableEntryFNDX, sizeof(OneStoreGlobalIdTableEntryFNDX), 1, file);
			
			cout << "FileNodeID: globalIdTableEntryFNDX:" << endl;
			print_guid(fnd.globalIdTableEntryFNDX.guid);
			cout << endl;
			
			break;
		}
		case FILENODEID_GLOBALIDTABLEENDFNDX: {
			// no data
			break;
		}
		case FILENODEID_DATASIGNATUREGROUPDEFINITIONFND: {
			/* [ File Node Header ][ DataSignatureGroupDefinitionFND ] */
			/*					=======^						 */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.dataSignatureGroupDefinitionFND, sizeof(OneStoreDataSignatureGroupDefinitionFND), 1, file);
			
			cout << "FileNodeID: dataSignatureGroupDefinitionFND:" << endl;
			print_extended_guid(fnd.dataSignatureGroupDefinitionFND.DataSignatureGroup);
			cout << endl;
			break;
		}
		case FILENODEID_OBJECTDECLARATIONWITHREFCOUNTFNDX: {
			fnd.objectDeclarationWithRefCountFNDX.ObjectRef = get_actual_file_node_chunk_reference(file, fileNode);
			/* [ File Node Header ][[ ObjectRef:FileNodeChunkRef ][ body:ObjectDeclarationWithRefCountBody ][ cRef:byte ]] */
			/*													=======^												   */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader) + fileNode.extra.fileNodeChunkRefSize, SEEK_SET);
			fread(&fnd.objectDeclarationWithRefCountFNDX.data, sizeof(OneStoreObjectDeclarationWithRefCountFNDXData), 1, file);
			fileNode.extra.otherRefFlag = 1;
			break;
		}
		case FILENODEID_OBJECTINFODEPENDENCYOVERRIDESFND: {
			fnd.objectInfoDependencyOverridesFND.ref = get_actual_file_node_chunk_reference(file, fileNode);
			
			// read the header if the data part is available
			if(fnd.objectInfoDependencyOverridesFND.ref.stp == SET_UINT64 && fnd.objectInfoDependencyOverridesFND.ref.cb == 0) {
				fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader) + fileNode.extra.fileNodeChunkRefSize, SEEK_SET);
				fread(&fnd.objectInfoDependencyOverridesFND.data.header, sizeof(OneStoreObjectInfoDependencyOverrideHeader), 1, file);
			}
			
			// read the data overrides arrays
			
			// TODO: special methods to fetch array of data overrides
			
			fileNode.extra.otherRefFlag = 1;
			break;
		}
		case FILENODEID_ROOTOBJECTREFERENCE2FNDX: {
			/* [ File Node Header ][ RootObjectReference2FNDX ] */
			/*					=======^						*/
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.rootObjectReference2FNDX, sizeof(OneStoreRootObjectReference2FNDX), 1, file);
			
			break;
		}
		case FILENODEID_REVISIONMANIFESTENDFND: {
			break;
		}
		case FILENODEID_CHUNKTERMINATORFND: {
			break;
		}
		case FILENODEID_GLOBALIDTABLEENTRY2FNDX: {
			/* [ File Node Header ][ GlobalIdTableEntry2FNDX ] */
			/*					=======^					   */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader), SEEK_SET);
			fread(&fnd.globalIdTableEntry2FNDX, sizeof(OneStoreGlobalIdTableEntry2FNDX), 1, file);
			break;
		}
		case FILENODEID_OBJECTREVISIONWITHREFCOUNTFNDX: {
			fnd.objectRevisionWithRefCountFNDX.ref = get_actual_file_node_chunk_reference(file, fileNode);
			/* [ File Node Header ][[ ref:FileNodeChunkRef ][ data:ObjectRevisionWithRefCountFNDX ]] */
			/*													=======^							 */
			fseek(file, fileNode.extra.stp + sizeof(OneStoreFileNodeHeader) + fileNode.extra.fileNodeChunkRefSize, SEEK_SET);
			fread(&fnd.objectRevisionWithRefCountFNDX.data, sizeof(OneStoreObjectRevisionWithRefCountFNDXData), 1, file);
			fileNode.extra.otherRefFlag = 1;
			break;
		}
		default:
			break;
	}
	
	return fnd;
	
	// TODO: for now, complete the struct forming for reading the TOC..
}

void conote::OneStore::copy_fnd_to_struct(uint8_t* fnd, void* structRef, size_t structSize, void** outFnd, size_t* outStructSize, OneStoreFileNode& fileNode)
{
	memcpy(structRef, fnd, structSize);
	*outFnd = structRef;
	*outStructSize = structSize;
}

void conote::OneStore::get_next_fragment_ref(FILE* file, OneStoreFileNodeListFragment &frag)
{
	if(file == NULL)
		return;
	OneStoreFileChunkReference64x32 ref;
	uint64_t magicNumber;
	fseek(file, frag.extra.footerMagicNumberStartIndex - sizeof(OneStoreFileChunkReference64x32), SEEK_SET);
	fread(&ref, sizeof(OneStoreFileChunkReference64x32), 1, file);
	fread(&magicNumber, sizeof(uint64_t), 1, file);
	frag.footer = magicNumber;
	frag.nextFragment = ref;
}

std::vector<OneStoreFileNode> conote::OneStore::get_file_nodes(FILE* file, OneStoreFileNodeListFragment& frag, uint64_t fragmentStp, uint32_t fragCb, uint recursiveFlag)
{

	// read all FileNodes
	uint64_t fnSeekPtr = 0, fileSize = 0;
	uint8_t loopFlag = 1;
	
	// know file size
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	
	// seek to the end of FNLFragHeader
	fnSeekPtr = fragmentStp + sizeof(OneStoreFileNodeListHeader);
	fseek(file, fnSeekPtr, SEEK_SET);
	
	vector<OneStoreFileNode> rgFileNodes;
	
	while(loopFlag) {
		OneStoreFileNode fileNode;
		fread(&fileNode.header, sizeof(OneStoreFileNodeHeader), 1, file);
		
		if(fileNode.header.Size > 0) {
			
			BOOST_LOG_TRIVIAL(debug) << "Fnd size: " << fileNode.header.Size - sizeof(OneStoreFileNodeHeader);

			BOOST_LOG_TRIVIAL(debug) << "Dumping fnd data:" << endl
			<< "OneStoreFileNodeHeader.fileNodeID: "<< std::hex << fileNode.header.fileNodeID << endl << std::dec
//			<< "OneStoreFileNodeHeader.Size: "<< fileNode.header.Size << endl
//			<< "OneStoreFileNodeHeader.aStpFormat: "<< fileNode.header.aStpFormat << endl
//			<< "OneStoreFileNodeHeader.bCbFormat: "<< fileNode.header.bCbFormat << endl
//			<< "OneStoreFileNodeHeader.cBaseType: "<< fileNode.header.cBaseType << endl
//			<< "OneStoreFileNodeHeader.dReserved: "<< fileNode.header.dReserved << endl
//			<< endl
//			<< "End dump"
			<< endl;
			
			fileNode.extra.stp = fnSeekPtr;
			
			if(recursiveFlag) {
				fileNode.fnd = get_actual_file_node_struct(file, fileNode);
			}
			
			rgFileNodes.push_back(fileNode);
			
		} else {
			loopFlag = 0;
			BOOST_LOG_TRIVIAL(debug) << "Broke by header size.";
			break;
		}
		
		fnSeekPtr += fileNode.header.Size;
		fseek(file, fnSeekPtr, SEEK_SET);
		
		if(fileSize <= fnSeekPtr /*|| fnSeekPtr - fragmentStp > fragCb*/) {
			loopFlag = 0;
			BOOST_LOG_TRIVIAL(debug) << "Broke by out of bounds: " << fnSeekPtr;
			break;
		}
		
		if(frag.extra.footerMagicNumberStartIndex - (fnSeekPtr + 12 + sizeof(OneStoreFileNodeHeader)) < 4) {
			loopFlag = 0;
			BOOST_LOG_TRIVIAL(debug) << "Broke by reaching footer.";
			break;
		}
		
		if(fileNode.header.fileNodeID == FILENODEID_CHUNKTERMINATORFND) {
			loopFlag = 0;
			BOOST_LOG_TRIVIAL(debug) << "Broke due to filenode chunk term fnd.";
			break;
		}
		// clean up
		
	}
	
	return rgFileNodes;
}
void conote::OneStore::print_guid(GUID guid)
{
	printf("Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}", 
	  guid.Data1, guid.Data2, guid.Data3, 
	  guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
	  guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}
void conote::OneStore::print_extended_guid(ExtendedGUID eguid)
{
	GUID guid = eguid.guid;
	printf("Guid = {{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX},%u}", 
	  guid.Data1, guid.Data2, guid.Data3, 
	  guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
	  guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7], eguid.n);
}

vector<CompactID> conote::OneStore::get_object_space_object_some_ids(FILE* file, OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, uint64_t memberStpOffset, uint64_t& outNextIDsOffset, OneStoreObjectSpaceObjectStreamHeader& emptyHeader)
{
	vector<CompactID> body;
	size_t byteCtr = 0; // to count the total bytes taken to read the field and also use the count as offset for next field of the ObjectSpaceObjectPropSet struct
	if(file == NULL)
		return body;
	
	// this loop is not much required because the next field to be read is determined by the flags: ExtendedStreamsPresent and OsidStreamNotPresent
	do {
		
		// break if the (incremented) offset out of bounds of the ObjectSpaceObjectPropSet structure.
		if(memberStpOffset >= objectSpaceObjPropSetRef.cb) {
			return body;
		}
	
		fseek(file, objectSpaceObjPropSetRef.stp + memberStpOffset, SEEK_SET);
		
		fread(&emptyHeader, sizeof(OneStoreObjectSpaceObjectStreamHeader), 1, file);
		byteCtr += sizeof(OneStoreObjectSpaceObjectStreamHeader);
		
		if(emptyHeader.Reserved != 0 || (emptyHeader.Count * sizeof(CompactID)) > (objectSpaceObjPropSetRef.cb - memberStpOffset)) {
			BOOST_LOG_TRIVIAL(error) << "Skipping to next field since Reserved field is not zero and the object count is astronomical: " << (emptyHeader.Count * 1);
			outNextIDsOffset = memberStpOffset;
			// increment to skip a byte and see if next stream is present
			memberStpOffset++;
		}
	
	} while(emptyHeader.Reserved != 0); // repeat until you see Reserved is zero
	
	for(int i = 0; i < emptyHeader.Count; i++) {
		CompactID id;
		fread(&id, sizeof(CompactID), 1, file);
		body.push_back(id);
		byteCtr += sizeof(CompactID);
	}
	
	// inform about possible offset of the next field of the ObjectSpaceObjectPropSet structure
	outNextIDsOffset = byteCtr;
	
	return body;
}

vector<CompactID> conote::OneStore::get_object_space_object_oids(FILE* file, OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, OneStoreObjectSpaceObjectPropSet& emptyPropSet, OneStoreObjectSpaceObjectStreamHeader& emptyHeader)
{
	uint64_t nextOffset = 0;
	vector<CompactID> body = get_object_space_object_some_ids(file, objectSpaceObjPropSetRef, 0, nextOffset, emptyHeader);
	
	BOOST_LOG_TRIVIAL(debug) << "Read oids: " << emptyHeader.Count * 1;
		
	if(!emptyHeader.OsidStreamNotPresent)
		emptyPropSet.extra.osidsOffset = nextOffset;
	else
		emptyPropSet.extra.propSetOffset = nextOffset;
	return body;
}
vector<CompactID> conote::OneStore::get_object_space_object_osids(FILE* file, OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, uint64_t memberStpOffset, OneStoreObjectSpaceObjectPropSet& propSet, OneStoreObjectSpaceObjectStreamHeader& emptyHeader)
{
	uint64_t nextOffset = 0;
	vector<CompactID> body = get_object_space_object_some_ids(file, objectSpaceObjPropSetRef, propSet.extra.osidsOffset, nextOffset, emptyHeader);
	
	BOOST_LOG_TRIVIAL(debug) << "Read osids: " << emptyHeader.Count * 1;
	
	if(emptyHeader.ExtendedStreamsPresent)
		propSet.extra.contextIdsOffset = nextOffset;
	else
		propSet.extra.propSetOffset = nextOffset;
	return body;
}
vector<CompactID> conote::OneStore::get_object_space_object_contextids(FILE* file, OneStoreFileNodeChunkReference objectSpaceObjPropSetRef, uint64_t memberStpOffset, OneStoreObjectSpaceObjectPropSet& propSet, OneStoreObjectSpaceObjectStreamHeader& emptyHeader)
{
	uint64_t nextOffset = 0;
	vector<CompactID> body = get_object_space_object_some_ids(file, objectSpaceObjPropSetRef, propSet.extra.contextIdsOffset, nextOffset, emptyHeader);
	
	BOOST_LOG_TRIVIAL(debug) << "Read ctxids: " << emptyHeader.Count * 1;
	
	propSet.extra.propSetOffset = nextOffset;
	return body;
}
void conote::OneStore::get_propset_fields(FILE* file, 
	uint64_t objectSpaceObjPropSetRef, uint64_t propSetOffset, 
	size_t& outPropertiesCount, vector<OneStorePropertyID>& rgPrids, 
	vector<conote::onestore::PropSetVal>& rgData, conote::onestore::PropSet& propSetRefObj, 
		vector<CompactID> oids,
		vector<CompactID> osids,
		vector<CompactID> ctxIds)
{
	if(file == NULL)
		return;
	
	conote::onestore::PropSet propSet;
	propSet.parse_propset_from_stream(file, objectSpaceObjPropSetRef, propSetOffset, oids, osids, ctxIds);
	outPropertiesCount = propSet.get_properties_count();
	rgPrids = propSet.get_prids();
	rgData = propSet.get_data();
	propSetRefObj = propSet;
	
}
JCID conote::OneStore::get_jcid_by_jci(uint jci)
{
	JCID jcid;
	jcid.index = jci;
	jcid.IsBinary = 0;
	jcid.IsFileData = 0;
	jcid.IsGraphNode = 0;
	jcid.IsPropertySet = 1;
	jcid.Reserved = 0;
	jcid.IsReadOnly = 0;
	return jcid;
}
uint32_t conote::OneStore::jcid_to_uint32(JCID jcid)
{
	uint32_t* jcidUint = (uint32_t*) &jcid;
	return *jcidUint;
}
